import axios from 'axios';

export async function getEventList() {
  const { data } = await axios.get('http://api.v.noinfinity.top/mock/event');
  return data;
}