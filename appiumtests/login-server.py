# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>


from flask import Flask, request, abort
app = Flask(__name__)

@app.route("/_matrix/client/v3/login", methods=["GET"])
def login_get():
    result = dict()
    result["flows"] = [dict()]
    result["flows"][0]["type"] = "m.login.password"
    return result

@app.route("/_matrix/client/v3/login", methods=["POST"])
def login_post():
    data = request.get_json()
    if data["identifier"]["user"] != "user" or data["password"] != "1234":
        abort(403)
    print(data)
    result = dict()
    result["access_token"] = "token_1234"
    result["device_id"] = "device_1234"
    result["user_id"] = "@user:localhost:1234"
    return result

@app.route("/_matrix/client/r0/sync")
def sync():
    result = dict()
    result["next_batch"] = "batch1234"
    return result

@app.route("/.well-known/matrix/client")
def well_known():
    reply = dict()
    reply["m.homeserver"] = dict()
    reply["m.homeserver"]["base_url"] = "https://localhost:1234"
    return reply


if __name__ == "__main__":
    app.run(ssl_context='adhoc', port=1234)
