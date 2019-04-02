import requests

TOKEN = "NTYyNjM1ODk5MzE0MzcyNjE4.XKNpig.QAiFqpcua1ojloNuVzEJtzJoqiU"
#testaccount123@dispostable.com test123
auth_url = "https://discordapp.com/api/v6/users/@me"
basic_auth_headers = {
    "Authorization": f"Bot {TOKEN}"
}

response = requests.get(auth_url, headers=basic_auth_headers)

print(response.status_code, response.json())

print()
print()
print('____________________________________')
print()
print()

response = requests.get("wss://gateway.discord.gg/")
print(response.status_code, response.json())


print()
print()
print('____________________________________')
print()
print()


channel_id = 562636135428521986
send_url = f"https://discordapp.com/api/v6/channels/{channel_id}/messages"
basic_auth_headers['Content-Type'] = "application/json"
data = {
  "content": "Hello, World!",
  "tts": False#,
#   "embed": {
#     "title": "Hello, Embed!",
#     "description": "This is an embedded message."
#   }
}


response = requests.post(send_url, json=data, headers=basic_auth_headers)

print(response.status_code, response.json())