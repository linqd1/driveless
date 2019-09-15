from channels.generic.websocket import WebsocketConsumer
import json

from django.core.cache import cache

class VEventConsumer(WebsocketConsumer):
  def connect(self):
    self.accept()

  def disconnect(self, close_code):
    pass

  def receive(self, text_data):
    text_data_json = json.loads(text_data)
    public = text_data_json['public']
    cache.set(public, self.channel_name, None)

  def vevent_send(self, event):
    self.send(text_data=json.dumps(event['data']))