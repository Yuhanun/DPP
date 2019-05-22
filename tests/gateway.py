import requests

TOKEN = "NTYyNjM1ODk5MzE0MzcyNjE4.XKNpig.QAiFqpcua1ojloNuVzEJtzJoqiU"
#testaccount123@dispostable.com test123
auth_url = "https://discordapp.com/api/v6/voice/regions"
basic_auth_headers = {
    "Authorization": f"Bot {TOKEN}",
    "Content-Type": "application/json",
    "User-Agent": "DiscordPP (http://www.github.com/yuhanun/dpp, 0.0.0)",
    "Connection": "keep-alive"
}

response = requests.get(auth_url, headers=basic_auth_headers)

print(response.status_code, response.json())

print('____________________________________')
