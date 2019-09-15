#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <algorithm>
#include <sstream>
#include <memory>
#include <iostream>
#include "mychainlib/mapping.hpp"
#include <vector>
#include <mychainlib/contract.h>
#define Interface struct
#define INTERFACE [[mychain::interface]] void
extern FILE *const stdin;
using namespace mychain;

class Bigint
{
    public:
        Bigint();
        Bigint(long long num);
        Bigint(const std::string& num);
        Bigint(const Bigint& other);
        ~Bigint()=default;

        friend std::ostream& operator << (std::ostream& os, Bigint num); 
        friend std::istream& operator >> (std::istream& is, Bigint& num);
        Bigint& operator = (const Bigint& other);
        Bigint operator + (Bigint other);
        Bigint operator * (const Bigint& other);
        Bigint operator - (const Bigint& other);
        Bigint operator / (const Bigint& other);
        int abscompare (const Bigint& other);
        bool operator < (const Bigint& other);
        bool operator > (const Bigint& other);
        bool operator <= (const Bigint& other);
        bool operator >= (const Bigint& other);
        bool operator == (const Bigint& other);
        bool operator != (const Bigint& other);
    private:
        Bigint operator * (const int s);
        void squeeze();
    private:
        std::string _num;                 //存储数据
        int sign;                         //符号位
};



class drivelesscar:public Contract {
  private:
    int32_t count;
    STORAGE(drivelesscar, (count))

  public:
    uint32_t m;
    INTERFACE Init(uint32_t m) {
       count = m;  // 将 count 初始化为 m 变量传入值
    }
};


class Account {
  std::string username= "";
  std::string bio="";

  int createdAt = 0;
};

class Car {
   std::string id= "";// sha256(${transactionId}${timestamp}${OwnerId}${title}${url})
   std::string transactionId= "";
   std::string  OwnerId= "";
   int   timestamp= 0;
   std::string title= "";
   std::string  url = "";
   int rewardCount= 0;
   int StatusCount= 0;
   int reportCount= 0;
   int score= 0;
};

class Status {
  std::string id= "" ;// sha256(${transactionId}${timestamp}${OwnerId}${CarId})
  std::string  transactionId= "";
   int timestamp= 0;
  std::string   OwnerId = "";
  std::string  CarId= "";
  std::string  content= "";
  int  reportCount= 0;
};
Interface AccountInfo{
  std::string username;
  std::string bio;
  int createdAt;
};

Interface CarInfo{
  std::string id;
  std::string transactionId;
  AccountInfo Owner;
  int  timestamp;
  std::string title;
  std::string url;
  int rewardCount;
  int StatusCount;
  int reportCount;
  int score;
};

Interface StatusInfo {
   std::string id;
  std::string transactionId;
  int  timestamp;
  AccountInfo  Owner;
 std::string   CarId;
   std::string content;
   int reportCount;
};
class drivelesscarContract:public Contract{
   std::string genesisAddress;
   MAPPING<int,std::string> holding;
  int reportThreshold;
  MAPPING<int,Account> accounts;// { accountId: account }
  MAPPING<int,std::string> usernames;// { username: accountId }
  std::vector<Car>  Cars;// [Car, ...], 按时间升序
  MAPPING<int,std::string>  CarIndexes;// { CarId: 0（车对应Cars中的下标） }
  MAPPING<std::vector<Status>> Status;// { CarId: [Status, ...] }, Status 时间升序
  MAPPING<MAPPING<int>> CarReports;// { CarId: { reporterId: 1 } }, report 时间升序
  MAPPING<MAPPING<int>>  StatusReports;// { StatusId: { reporterId: 1 } }, report 时间升序

   void constructor() {
     void super()

    genesisAddress = "GENESIS_ADDRESS"
    holding = new MAPPING<Bigint>()

    reportThreshold = 3
    accounts = new MAPPING<Account>()
    usernames = new MAPPING<std::string>()
    Cars = new vector<Car>()
    CarIndexes = new MAPPING<int>()
    Status = new MAPPING<std::vector<Status>>()
    CarReports = new MAPPING<MAPPING<int>>()
    StatusReports = new MAPPING<MAPPING<int>>()
  };
};


  void onPay (Bigint amount, std::string currency) {
    assert(amount > 0  "Amount must great than 0")
    assert(currency === "CART"  "Support CART only")

    const senderAddress = context.senderAddress

    increaseHolding(senderAddress, amount)
  };


   void onPayout (int amount) {
    assert(amount > 0 "amount must greater than 0")
    const senderAddress = context.senderAddress
    const holding = getHolding(senderAddress)
    assert((holding > 0) && (amount <= holding) "CART not enough for payout")

    // 1. 减去用户 holding
    increaseHolding(senderAddress, -amount)


    transfer(senderAddress, amount, "CART")
  };

  void increaseHolding (std::string address, Bigint | int value) {
    const holdingValue = holding[address] || Bigint(0)
    holding[address] = holdingValue + Bigint(value)
  };

   Bigint getHolding (std::string senderAddress) {
    return holding[senderAddress] || Bigint(0)
  }
  
   void createOrUpdateAccount (std::string username, std::string bio){
    assert(username && (username.length < 50) 'Please set your username with 50 characters')
    assert(bio && (bio.length < 256) 'Please set your bio with 256 characters')

    const senderAddress = context.senderAddress
    const _accountId = usernames[username]
    
    if (_accountId) {
      assert(_accountId === senderAddress 'This username already exists')
    }

    const holding = getHolding(std::string senderAddress)
    const fee = Bigint(100)
    assert(holding > fee, 'CART not enough for create or update account')
    increaseHolding(senderAddress, -fee)

   
    const account = accounts[senderAddress]
    if (account) {
     
      usernames[account.username] = undefined
      account.username = username
      account.bio = bio
      // 新的username映射
      usernames[username] = senderAddress
    } else {
      // 创建用户
      accounts[senderAddress] = {
        username,
        bio,
        createdAt: context.transaction.timestamp
      }
      usernames[username] = senderAddress
    }
  }


    int calcScore (Car | CarInfo Car){
      let elapsedHours = (context.transaction.timestamp - Car.timestamp) / 3600000
      return Math.sqrt(Car.rewardCount + Car.StatusCount + 1) /
        Math.pow(elapsedHours + Car.reportCount * 2 + 2, 1.8)
    }


    void createCar (std::string title, std::string url){
      const senderAddress = context.senderAddress
      const account = accounts[senderAddress]
      assert(account, 'Please create an account first')
      assert(title, 'Missing title')
      assert(title.length < 256, 'Title must less or equal than 256 characters')
      assert(url, 'Missing url')
      assert(url.length < 256, 'Url must less or equal than 256 characters')

      const lastCar = getAccountLastCar(std::string senderAddress)
      if (lastCar) {
        assert(context.transaction.timestamp - lastCar.timestamp < 8000, 'create Car too frequently')
      }<
      const transactionId = context.transaction.id
      const OwnerId = context.senderAddress
      const timestamp = context.transaction.timestamp
      const Car = {
        id: Crypto.sha256.hash(`${transactionId}${timestamp}${OwnerId}${title}${url}`),
        transactionId,
        OwnerId,
        timestamp,
        title,
        url,
        rewardCount: 0,
        StatusCount: 0,
        reportCount: 0,
        score: 0
      }

  
      Cars.push(Car)

 
      CarIndexes[Car.id] = Cars.size() - 1

  
      const reward = Bigint(10)
      const genesisAddressHolding = getHolding(genesisAddress)
      assert(genesisAddressHolding > reward, 'CART pool not enough')

      increaseHolding(senderAddress, reward)
      increaseHolding(genesisAddress, -reward)
    }


    Car getOneCar (std::string CarId) {
      const index = CarIndexes[CarId]!
      assert(index !== undefined, 'Cannot find this Car')

      const Car = Cars[index]
      assert(Car, 'Cannot find this Car')

      return Car
    }


      Car getAccountLastCar (std::string accountId){
      let count = Cars.size()
      let Car: Car
      for (let i = count - 1; i >= 0; i--) {
        const _Car = Cars[i]!
        if (_Car.OwnerId === accountId) {
          Car = _Car
          break
        }
      }

      return Car
    }


    void rewardCar (std::string CarId, int amount){
      const senderAddress = context.senderAddress
      const account = accounts[senderAddress]
      assert(account, 'Please create an account first')
      assert(CarId, 'Missing CarId')
      const Car = getOneCar(CarId)
      // assert(Car, 'Cannot find this Car')
      assert(amount > 10, 'Amount must greater than 10')
      const remainingAmount = getHolding(senderAddress)
      assert(remainingAmount > amount, 'CART not enough for reward this Car')

      // 1. 先减去用户 CART
      increaseHolding(senderAddress, -amount)

      // 2. 增加车 rewardCount
      Car!.rewardCount += amount

      // 3. 打赏作者，10%手续费，记账
      const reward = Bigint(amount)
      const fee = Bigint(Math.floor(amount / 10))
      increaseHolding(senderAddress, -reward)
      increaseHolding(Car!.OwnerId, reward - fee)
      increaseHolding(genesisAddress, fee)
    }


    void reportCar (std::string CarId) {
      const senderAddress = context.senderAddress
      const account = accounts[senderAddress]
      assert(account, 'Please create an account first')
      assert(CarId, 'Missing CarId')
      const Car = getOneCar(CarId)
      assert(Car, 'Cannot find this Car')

      // 1. 创建 report 记录
      CarReports[CarId] = CarReports[CarId] || new MAPPING()

      assert(CarReports[CarId]![senderAddress] !== 1, 'You already reported this Car')
      CarReports[CarId]![senderAddress] = 1

      // 2. 车reportCount+1
      Car!.reportCount += 1
 
  
    CarInfo[] getCarsByTime (int limit, int offset){
      assert(limit > 0 && limit <= 100, 'limit must greater than 0 and less or equal to 100')
      assert(offset >= 0, 'offset must greater or equal than 0')

      let count = Cars.size()
      const Cars = []
      for (let i = Math.min(offset + limit, count) - 1; i >= offset; i--) {
        const Car: CarInfo = { ...Cars[i]! }
        if (Car.reportCount >= reportThreshold) {
          continue
        }
        Car.Owner = accounts[Cars[i]!.OwnerId]!
        Cars.push(Car)
      }

      return Cars
    }

    // 获取所有车(按热度降序，可翻页)
 
    CarInfo[] getCarsByScore(int limit, int offset) {
      assert(limit > 0 && limit <= 100, 'limit must greater than 0 and less or equal to 100')
      assert(offset >= 0, 'offset must greater or equal than 0')

      let count = Math.min(Cars.size(), 500)
      // 1. 先取 500 条最新的
      let Cars = []
      for (let i = Math.min(offset + limit, count) - 1; i >= offset; i--) {
        const Car: CarInfo = { ...Cars[i]! }
        if (Car.reportCount >= reportThreshold) {
          continue
        }
        Car.Owner = accounts[Cars[i]!.OwnerId]!
        Car.score = calcScore(Car)

        Cars.push(Car)
      }

      // 2. score 降序
      Cars = Cars.sort((prev, next) => {
        return next.score - prev.score
      })

      // 3. 截取
      Cars = Cars.slice(offset, offset + limit)

      return Cars
    }

    /*************** Status ***************/
  // 创建一个留言
  void createStatus (std::string CarId, std::string content){
    const senderAddress = context.senderAddress
    const account = accounts[context.senderAddress]
    assert(account, 'Please create an account first')
    assert(CarId, 'Missing CarId')
    const Car = getOneCar(CarId)
    assert(Car, 'Cannot find this Car')
    assert(content, 'Missing content')
    assert(content.length < 1024, 'Content must less or equal 1024 characters')

    const transactionId = context.transaction.id
    const timestamp = context.transaction.timestamp
    const Status = {
      id: Crypto.sha256.hash(`${transactionId}${timestamp}${senderAddress}${CarId}`),
      transactionId,
      timestamp,
      OwnerId: senderAddress,
      CarId,
      content,
      reportCount: 0
    }

    Status[CarId] = Status[CarId] || new Vector()
    Status[CarId]!.push(Status)
    Car.StatusCount += 1
  }

  // 获取一个车下所有留言(按创建时间降序，可翻页)

  StatusInfo[] getOneCarStatus (std::string CarId, int limit, int offset){
    assert(limit > 0 && limit <= 100, 'limit must greater than 0 and less or equal to 100')
    assert(offset >= 0, 'offset must greater or equal than 0')
    assert(CarId, 'Missing CarId')
    const Car = getOneCar(CarId)
    assert(Car, 'Cannot find this Car')

    let CarStatus = Status[CarId] || (new Vector())
    let count = CarStatus.size()

    const Status = []
    for (let i = Math.min(count, offset + limit) - 1; i >= offset; i--) {
      const Status: StatusInfo = { ...CarStatus[i]! }
      if (Status.reportCount >= reportThreshold) {
        continue
      }
      Status.Owner = accounts[CarStatus[i]!.OwnerId]!
      Status.push(Status)
    }

    return Status
  }

  // 获取一个留言
  Status getOneStatus (std::string CarId,std::string StatusId ){
    assert(CarId, 'Missing CarId')
    const Car = getOneCar(CarId)
    assert(Car, 'Cannot find this Car')

    let Status = Status[CarId] || new Vector()
    let count = Status.size()
    let Status: Status
    for (let i = count - 1; i >= 0; i--) {
      if (Status[i]!.id === StatusId) {
        Status = Status[i]!
        break
      }
    }
    return Status
  }

  // 举报一个留言
  void reportStatus (std::string CarId , std::string StatusId ) {
    const senderAddress = context.senderAddress
    const account = accounts[senderAddress]
    assert(account, 'Please create an account first')
    assert(CarId, 'Missing CarId')
    assert(StatusId, 'Missing StatusId')
    const Status = getOneStatus(CarId, StatusId)
    assert(Status, 'Cannot find this Status')

    // 1. 创建 report 记录
    StatusReports[StatusId] = StatusReports[StatusId] || new MAPPING()

    assert(StatusReports[StatusId]![senderAddress] !== 1, 'You already reported this Status')
    StatusReports[StatusId]![senderAddress] = 1

    // 2. 留言reportCount+1
    Status!.reportCount += 1
  }
