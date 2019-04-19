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