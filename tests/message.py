import requests

TOKEN = "NTYyNjM1ODk5MzE0MzcyNjE4.XKNpig.QAiFqpcua1ojloNuVzEJtzJoqiU"
#testaccount123@dispostable.com test123
basic_auth_headers = {
    "Authorization": f"Bot {TOKEN}",
    "Content-Type": "application/json"
}

channel_id = 562636135428521986
send_url = f"http://discordapp.com/api/v6/channels/{channel_id}/messages"
data = {
  "content": "Hello, World!",
  "tts": False,
   "embed": {
     "title": "Hello, Embed!",
     "description": "This is an embedded message."
   }
}

response = requests.post(send_url, json=data, headers=basic_auth_headers)

print(response.status_code, response.json())