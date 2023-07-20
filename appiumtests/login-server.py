# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>

import json
from flask import Flask, request, abort
import os
app = Flask(__name__)


@app.route("/_matrix/client/v3/login", methods=["GET"])
def login_get():
    result = dict()
    result["flows"] = [dict()]
    result["flows"][0]["type"] = "m.login.password"
    return result

@app.route("/_matrix/client/v3/account/whoami", methods=["GET"])
def whoami():
    result = dict()
    result["device_id"] = "device_id_1234"
    result["user_id"] = "@user:localhost:1234"
    return result

@app.route("/_matrix/client/v3/login", methods=["POST"])
def login_post():
    data = request.get_json()
    if data["identifier"]["user"] != "user" or data["password"] != "1234":
        abort(403)
    print(data)
    result = dict()
    result["access_token"] = "token_login"
    result["device_id"] = "device_1234"
    result["user_id"] = "@user:localhost:1234"
    return result

def load_json(name):
    parts = __file__.split("/")
    parts.pop()
    datadir = "/".join(parts)
    return json.loads(open(f"{datadir}/data/{name}.json").read())


@app.route("/_matrix/client/r0/sync")
def sync():

    result = load_json("sync_response_no_rooms") if ("login" in request.headers.get("Authorization")) else load_json("sync_response_rooms")
    return result

@app.route("/.well-known/matrix/client")
def well_known():
    reply = dict()
    reply["m.homeserver"] = dict()
    reply["m.homeserver"]["base_url"] = "https://localhost:1234"
    return reply

@app.route("/_matrix/client/v3/profile/<id>")
def profile(id):
    reply = dict()
    reply["avatar_url"] = "mxc://localhost:1234/asdf1234"
    reply["displayname"] = "User123"
    return reply

@app.route("/_matrix/client/v3/keys/upload", methods=["POST"])
def upload_keys():
    reply = dict()
    return reply


if __name__ == "__main__":
    app.run(ssl_context='adhoc', port=1234)
