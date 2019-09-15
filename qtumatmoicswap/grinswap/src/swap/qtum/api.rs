use super::client::grinNodeClient;
use super::types::{grinBuyerContext, grinData, grinSellerContext};
use crate::swap::message::{Message, Update};
use crate::swap::types::{
	Action, BuyerContext, Context, Currency, Role, RoleContext, SecondaryBuyerContext,
	SecondarySellerContext, SellerContext, Status,
};
use crate::swap::{BuyApi, ErrorKind, Keychain, SellApi, Swap, SwapApi};
use qtum::{Address, AddressType};
use grin_keychain::{Identifier, SwitchCommitmentType};
use grin_util::secp::aggsig::export_secnonce_single as generate_nonce;
use libwallet::NodeClient;
use std::str::FromStr;
use std::time::Duration;

pub struct grinSwapApi<K, C, B>
where
	K: Keychain,
	C: NodeClient,
	B: grinNodeClient,
{
	keychain: Option<K>,
	node_client: C,
	grin_node_client: B,
}

impl<K, C, B> grinSwapApi<K, C, B>
where
	K: Keychain,
	C: NodeClient,
	B: grinNodeClient,
{
	/// Create grin Swap API instance
	pub fn new(keychain: Option<K>, node_client: C, grin_node_client: B) -> Self {
		Self {
			keychain,
			node_client,
			grin_node_client,
		}
	}

	fn keychain(&self) -> Result<&K, ErrorKind> {
		match &self.keychain {
			Some(k) => Ok(k),
			None => Err(ErrorKind::MissingKeychain),
		}
	}

	fn script(&self, swap: &mut Swap) -> Result<(), ErrorKind> {
		let grin_data = swap.secondary_data.unwrap_grin_mut()?;
		grin_data.script(
			self.keychain()?.secp(),
			swap.redeem_public
				.as_ref()
				.ok_or(ErrorKind::UnexpectedAction)?,
		)?;
		Ok(())
	}

	fn grin_balance(
		&mut self,
		swap: &mut Swap,
		confirmations_needed: u64,
	) -> Result<(u64, u64, u64), ErrorKind> {
		self.script(swap)?;
		let grin_data = swap.secondary_data.unwrap_grin_mut()?;
		let address = grin_data.address(swap.network)?;
		let outputs = self.grin_node_client.unspent(&address)?;
		let height = self.grin_node_client.height()?;
		let mut pending_amount = 0;
		let mut confirmed_amount = 0;
		let mut least_confirmations = None;

		let mut confirmed_outputs = Vec::new();

		for output in outputs {
			if output.height == 0 {
				// Output in mempool
				least_confirmations = Some(0);
				pending_amount += output.value;
			} else {
				let confirmations = height.saturating_sub(output.height) + 1;
				if confirmations >= confirmations_needed {
					// Enough confirmations
					confirmed_amount += output.value;
					confirmed_outputs.push(output);
				} else {
					// Not yet enough confirmations
					if least_confirmations
						.map(|least| confirmations < least)
						.unwrap_or(true)
					{
						least_confirmations = Some(confirmations);
					}
					pending_amount += output.value;
				}
			}
		}
		grin_data.confirmed_outputs = confirmed_outputs;

		Ok((
			pending_amount,
			confirmed_amount,
			least_confirmations.unwrap_or(0),
		))
	}

	// Seller specific methods

	/// Seller checks Grin and qtum chains for the locked funds
	fn seller_check_locks(&mut self, swap: &mut Swap) -> Result<Option<Action>, ErrorKind> {
		// Check Grin chain
		if !swap.is_locked(30) {
			match swap.lock_confirmations {
				None => return Ok(Some(Action::PublishTx)),
				Some(_) => {
					let confirmations =
						swap.update_lock_confirmations(self.keychain()?.secp(), &self.node_client)?;
					if !swap.is_locked(30) {
						return Ok(Some(Action::Confirmations {
							required: 30,
							actual: confirmations,
						}));
					}
				}
			};
		}

		// Check qtum chain
		if !swap.secondary_data.unwrap_grin()?.locked {
			let (pending_amount, confirmed_amount, mut least_confirmations) =
				self.grin_balance(swap, 6)?;
			if pending_amount + confirmed_amount < swap.secondary_amount {
				least_confirmations = 0;
			};

			if confirmed_amount < swap.secondary_amount {
				return Ok(Some(Action::ConfirmationsSecondary {
					required: 6,
					actual: least_confirmations,
				}));
			}

			swap.secondary_data.unwrap_grin_mut()?.locked = true;
		}

		// If we got here, funds have been locked on both chains with sufficient confirmations
		swap.status = Status::Locked;

		Ok(None)
	}

	/// Seller applies an update message to the Swap
	fn seller_receive_message(
		&self,
		swap: &mut Swap,
		context: &Context,
		message: Message,
	) -> Result<(), ErrorKind> {
		match swap.status {
			Status::Offered => self.seller_accepted_offer(swap, context, message),
			Status::Locked => self.seller_init_redeem(swap, context, message),
			_ => Err(ErrorKind::UnexpectedMessageType),
		}
	}

	/// Seller applies accepted offer message from buyer to the swap
	fn seller_accepted_offer(
		&self,
		swap: &mut Swap,
		context: &Context,
		message: Message,
	) -> Result<(), ErrorKind> {
		let (_, accept_offer, secondary_update) = message.unwrap_accept_offer()?;
		let grin_update = secondary_update.unwrap_grin()?.unwrap_accept_offer()?;

		SellApi::accepted_offer(self.keychain()?, swap, context, accept_offer)?;
		let grin_data = swap.secondary_data.unwrap_grin_mut()?;
		grin_data.accepted_offer(grin_update)?;

		Ok(())
	}

	/// Seller applies accepted offer message from buyer to the swap
	fn seller_init_redeem(
		&self,
		swap: &mut Swap,
		context: &Context,
		message: Message,
	) -> Result<(), ErrorKind> {
		let (_, init_redeem, _) = message.unwrap_init_redeem()?;
		SellApi::init_redeem(self.keychain()?, swap, context, init_redeem)?;

		Ok(())
	}

	/// Seller builds the transaction to redeem their qtums
	fn seller_build_redeem_tx(&self, swap: &mut Swap, context: &Context) -> Result<(), ErrorKind> {
		swap.expect(Status::Redeem)?;
		self.script(swap)?;
		let cosign_id = &context.unwrap_seller()?.unwrap_grin()?.cosign;

		let redeem_address = Address::from_str(&swap.unwrap_seller()?.0)
			.map_err(|_| ErrorKind::Generic("Unable to parse grin redeem address".into()))?;

		let cosign_secret =
			self.keychain()?
				.derive_key(0, cosign_id, &SwitchCommitmentType::None)?;
		let redeem_secret = SellApi::calculate_redeem_secret(self.keychain()?, swap)?;

		// This function should only be called once
		let grin_data = swap.secondary_data.unwrap_grin_mut()?;
		if grin_data.redeem_tx.is_some() {
			return Err(ErrorKind::OneShot)?;
		}

		grin_data.redeem_tx(
			self.keychain()?.secp(),
			&redeem_address,
			10,
			&cosign_secret,
			&redeem_secret,
		)?;
		swap.status = Status::RedeemSecondary;

		Ok(())
	}

	fn seller_update_redeem(&mut self, swap: &mut Swap) -> Result<Action, ErrorKind> {
		swap.expect(Status::RedeemSecondary)?;

		// We have generated the grin redeem tx..
		let grin_data = swap.secondary_data.unwrap_grin_mut()?;
		let txid = &grin_data
			.redeem_tx
			.as_ref()
			.ok_or(ErrorKind::Generic("Redeem transaction missing".into()))?
			.txid;

		if grin_data.redeem_confirmations.is_none() {
			// ..but we haven't published it yet
			Ok(Action::PublishTxSecondary)
		} else {
			// ..we published it..
			if let Some((Some(height), _)) = self.grin_node_client.transaction(txid)? {
				let confirmations = self.grin_node_client.height()?.saturating_sub(height) + 1;
				grin_data.redeem_confirmations = Some(confirmations);
				if confirmations > 0 {
					// ..and its been included in a block!
					return Ok(Action::Complete);
				}
			}
			// ..but its not confirmed yet
			Ok(Action::ConfirmationRedeemSecondary(format!("{}", txid)))
		}
	}

	// Buyer specific methods

	/// Buyer checks Grin and qtum chains for the locked funds
	fn buyer_check_locks(
		&mut self,
		swap: &mut Swap,
		context: &Context,
	) -> Result<Option<Action>, ErrorKind> {
		// Check qtum chain
		if !swap.secondary_data.unwrap_grin()?.locked {
			let (pending_amount, confirmed_amount, least_confirmations) =
				self.grin_balance(swap, 6)?;
			let chain_amount = pending_amount + confirmed_amount;
			if chain_amount < swap.secondary_amount {
				// At this point, user needs to deposit (more) qtum
				self.script(swap)?;
				return Ok(Some(Action::DepositSecondary {
					amount: swap.secondary_amount - chain_amount,
					address: format!(
						"{}",
						swap.secondary_data.unwrap_grin()?.address(swap.network)?
					),
				}));
			}

			// Enough confirmed or in mempool
			if confirmed_amount < swap.secondary_amount {
				// Wait for enough confirmations
				return Ok(Some(Action::ConfirmationsSecondary {
					required: 6,
					actual: least_confirmations,
				}));
			}

			swap.secondary_data.unwrap_grin_mut()?.locked = true;
		}

		// Check Grin chain
		let confirmations =
			swap.update_lock_confirmations(self.keychain()?.secp(), &self.node_client)?;
		if !swap.is_locked(30) {
			return Ok(Some(Action::Confirmations {
				required: 30,
				actual: confirmations,
			}));
		}

		// If we got here, funds have been locked on both chains with sufficient confirmations
		swap.status = Status::Locked;
		BuyApi::init_redeem(self.keychain()?, swap, context)?;

		Ok(None)
	}

	/// Buyer applies an update message to the Swap
	fn buyer_receive_message(
		&self,
		swap: &mut Swap,
		context: &Context,
		message: Message,
	) -> Result<(), ErrorKind> {
		match swap.status {
			Status::InitRedeem => self.buyer_redeem(swap, context, message),
			_ => Err(ErrorKind::UnexpectedMessageType),
		}
	}

	/// Buyer applies redeem message from seller to the swap
	fn buyer_redeem(
		&self,
		swap: &mut Swap,
		context: &Context,
		message: Message,
	) -> Result<(), ErrorKind> {
		let (_, redeem, _) = message.unwrap_redeem()?;
		BuyApi::redeem(self.keychain()?, swap, context, redeem)?;
		Ok(())
	}
}

impl<K, C, B> SwapApi<K> for grinSwapApi<K, C, B>
where
	K: Keychain,
	C: NodeClient,
	B: grinNodeClient,
{
	fn set_keychain(&mut self, keychain: Option<K>) {
		self.keychain = keychain;
	}

	fn context_key_count(
		&mut self,
		secondary_currency: Currency,
		_is_seller: bool,
	) -> Result<usize, ErrorKind> {
		if secondary_currency != Currency::grin {
			return Err(ErrorKind::UnexpectedCoinType);
		}

		Ok(4)
	}

	fn create_context(
		&mut self,
		secondary_currency: Currency,
		is_seller: bool,
		inputs: Option<Vec<(Identifier, u64)>>,
		keys: Vec<Identifier>,
	) -> Result<Context, ErrorKind> {
		if secondary_currency != Currency::grin {
			return Err(ErrorKind::UnexpectedCoinType);
		}

		let secp = self.keychain()?.secp();
		let mut keys = keys.into_iter();

		let role_context = if is_seller {
			RoleContext::Seller(SellerContext {
				inputs: inputs.ok_or(ErrorKind::UnexpectedRole)?,
				change_output: keys.next().unwrap(),
				refund_output: keys.next().unwrap(),
				secondary_context: SecondarySellerContext::grin(grinSellerContext {
					cosign: keys.next().unwrap(),
				}),
			})
		} else {
			RoleContext::Buyer(BuyerContext {
				output: keys.next().unwrap(),
				redeem: keys.next().unwrap(),
				secondary_context: SecondaryBuyerContext::grin(grinBuyerContext {
					refund: keys.next().unwrap(),
				}),
			})
		};

		Ok(Context {
			multisig_key: keys.next().unwrap(),
			multisig_nonce: generate_nonce(secp)?,
			lock_nonce: generate_nonce(secp)?,
			refund_nonce: generate_nonce(secp)?,
			redeem_nonce: generate_nonce(secp)?,
			role_context,
		})
	}

	/// Seller creates a swap offer
	fn create_swap_offer(
		&mut self,
		context: &Context,
		address: Option<String>,
		primary_amount: u64,
		secondary_amount: u64,
		secondary_currency: Currency,
		secondary_redeem_address: String,
	) -> Result<(Swap, Action), ErrorKind> {
		let redeem_address = Address::from_str(&secondary_redeem_address)
			.map_err(|_| ErrorKind::Generic("Unable to parse grin redeem address".into()))?;

		match redeem_address.address_type() {
			Some(AddressType::P2pkh) | Some(AddressType::P2sh) => {}
			_ => {
				return Err(ErrorKind::Generic(
					"Only P2PKH and P2SH grin redeem addresses are supported".into(),
				))
			}
		};

		if secondary_currency != Currency::grin {
			return Err(ErrorKind::UnexpectedCoinType);
		}

		let height = self.node_client.get_chain_height()?;
		let mut swap = SellApi::create_swap_offer(
			self.keychain()?,
			context,
			address,
			primary_amount,
			secondary_amount,
			Currency::grin,
			secondary_redeem_address,
			height,
		)?;

		let grin_data = grinData::new(
			self.keychain()?,
			context.unwrap_seller()?.unwrap_grin()?,
			Duration::from_secs(24 * 60 * 60),
		)?;
		swap.secondary_data = grin_data.wrap();

		let action = self.required_action(&mut swap, context)?;
		Ok((swap, action))
	}

	/// Buyer accepts a swap offer
	fn accept_swap_offer(
		&mut self,
		context: &Context,
		address: Option<String>,
		message: Message,
	) -> Result<(Swap, Action), ErrorKind> {
		let (id, offer, secondary_update) = message.unwrap_offer()?;
		let grin_data = grinData::from_offer(
			self.keychain()?,
			secondary_update.unwrap_grin()?.unwrap_offer()?,
			context.unwrap_buyer()?.unwrap_grin()?,
		)?;

		let height = self.node_client.get_chain_height()?;
		let mut swap =
			BuyApi::accept_swap_offer(self.keychain()?, context, address, id, offer, height)?;
		swap.secondary_data = grin_data.wrap();

		let action = self.required_action(&mut swap, context)?;
		Ok((swap, action))
	}

	fn completed(&mut self, swap: &mut Swap, context: &Context) -> Result<Action, ErrorKind> {
		match swap.role {
			Role::Seller(_, _) => {
				swap.expect(Status::RedeemSecondary)?;
				let grin_data = swap.secondary_data.unwrap_grin()?;
				if grin_data.redeem_confirmations.unwrap_or(0) > 0 {
					swap.status = Status::Completed;
				} else {
					return Err(ErrorKind::UnexpectedAction);
				}
			}
			Role::Buyer => BuyApi::completed(swap)?,
		}
		let action = self.required_action(swap, context)?;

		Ok(action)
	}

	fn refunded(&mut self, _swap: &mut Swap) -> Result<(), ErrorKind> {
		unimplemented!();
	}

	fn cancelled(&mut self, _swap: &mut Swap) -> Result<(), ErrorKind> {
		unimplemented!();
	}

	/// Check which action should be taken by the user
	fn required_action(&mut self, swap: &mut Swap, context: &Context) -> Result<Action, ErrorKind> {
		if swap.is_finalized() {
			return Ok(Action::None);
		}

		let action = match swap.role {
			Role::Seller(_, _) => {
				if swap.status == Status::Accepted {
					if let Some(action) = self.seller_check_locks(swap)? {
						return Ok(action);
					}
				} else if swap.status == Status::RedeemSecondary {
					return self.seller_update_redeem(swap);
				}
				let action = SellApi::required_action(&mut self.node_client, swap)?;

				match (swap.status, action) {
					(Status::Redeem, Action::Complete) => {
						self.seller_build_redeem_tx(swap, context)?;
						Action::PublishTxSecondary
					}
					(_, action) => action,
				}
			}
			Role::Buyer => {
				if swap.status == Status::Accepted {
					if let Some(action) = self.buyer_check_locks(swap, context)? {
						return Ok(action);
					}
				}
				BuyApi::required_action(&mut self.node_client, swap)?
			}
		};

		Ok(action)
	}

	fn message(&mut self, swap: &Swap) -> Result<Message, ErrorKind> {
		let message = match swap.role {
			Role::Seller(_, _) => {
				let mut message = SellApi::message(swap)?;
				if let Update::Offer(_) = message.inner {
					message.set_inner_secondary(
						swap.secondary_data.unwrap_grin()?.offer_update().wrap(),
					);
				}
				message
			}
			Role::Buyer => {
				let mut message = BuyApi::message(swap)?;
				if let Update::AcceptOffer(_) = message.inner {
					message.set_inner_secondary(
						swap.secondary_data
							.unwrap_grin()?
							.accept_offer_update()?
							.wrap(),
					);
				}
				message
			}
		};

		Ok(message)
	}

	/// Message has been sent to the counterparty, update state accordingly
	fn message_sent(&mut self, swap: &mut Swap, context: &Context) -> Result<Action, ErrorKind> {
		match swap.role {
			Role::Seller(_, _) => SellApi::message_sent(swap)?,
			Role::Buyer => BuyApi::message_sent(swap)?,
		}
		let action = self.required_action(swap, context)?;

		Ok(action)
	}

	/// Apply an update Message to the Swap
	fn receive_message(
		&mut self,
		swap: &mut Swap,
		context: &Context,
		message: Message,
	) -> Result<Action, ErrorKind> {
		if swap.id != message.id {
			return Err(ErrorKind::MismatchedId);
		}

		if swap.is_finalized() {
			return Err(ErrorKind::Finalized);
		}

		match swap.role {
			Role::Seller(_, _) => self.seller_receive_message(swap, context, message)?,
			Role::Buyer => self.buyer_receive_message(swap, context, message)?,
		};
		let action = self.required_action(swap, context)?;

		Ok(action)
	}

	fn publish_transaction(
		&mut self,
		swap: &mut Swap,
		context: &Context,
	) -> Result<Action, ErrorKind> {
		match swap.role {
			Role::Seller(_, _) => SellApi::publish_transaction(&self.node_client, swap),
			Role::Buyer => BuyApi::publish_transaction(&self.node_client, swap),
		}?;

		self.required_action(swap, context)
	}

	fn publish_secondary_transaction(
		&mut self,
		swap: &mut Swap,
		context: &Context,
	) -> Result<Action, ErrorKind> {
		swap.expect_seller()?;
		swap.expect(Status::RedeemSecondary)?;
		let grin_data = swap.secondary_data.unwrap_grin_mut()?;
		if grin_data.redeem_confirmations.is_some() {
			return Err(ErrorKind::UnexpectedAction);
		}

		let tx = grin_data
			.redeem_tx
			.as_ref()
			.ok_or(ErrorKind::UnexpectedAction)?
			.tx
			.clone();
		self.grin_node_client.post_tx(tx)?;
		grin_data.redeem_confirmations = Some(0);
		let action = self.required_action(swap, context)?;

		Ok(action)
	}
}
