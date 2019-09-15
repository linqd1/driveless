from django.shortcuts import render
from django.http import HttpResponse
from django.core.cache import cache
from channels.layers import get_channel_layer
from asgiref.sync import async_to_sync
import json

channel_layer = get_channel_layer()

# Create your views here.

def mockEventList(request):
  res = HttpResponse()
  res['Content-Type'] = 'application/json'
  res['Access-Control-Allow-Origin'] = 'http://v.noinfinity.top'
  res.content = '''
[
  {
    "type": {
      "level": "is-danger",
      "name": "超车"
    },
    "id": "0x123456789dead1",
    "status": "",
    "expire": 1555767918091
  },
  {
    "type": {
      "level": "is-primary",
      "name": "超车"
    },
    "id": "0x123456789dead2",
    "status": "",
    "expire": 123,
    "data": {
      "备注": "你好"
    }
  },
  {
    "type": {
      "level": "is-info",
      "name": "超车"
    },
    "id": "0x123456789dead3",
    "status": "accept",
    "decider": "car",
    "expire": 123
  },
  {
    "type": {
      "level": "is-danger",
      "name": "超车"
    },
    "id": "0x123456789dead4",
    "status": "reject",
    "decider": "human",
    "expire": 123,
    "from": "asd",
    "to": "bsd"
  }
]
'''
  return res

def eventList(request):
  if  request.method == 'POST':
    datas = json.loads(request.body)
    for data in datas:
      if cache.get(data['event']['to']):
        async_to_sync(channel_layer.send)(cache.get(data['event']['to']), {
            "type": "vevent.send",
            "data": data,
        })
    return HttpResponse()