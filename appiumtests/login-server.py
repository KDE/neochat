# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>

import json
from flask import Flask, request, abort
import os
app = Flask(__name__)

next_sync_payload = ""


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
    global next_sync_payload
    result = dict()
    if len(next_sync_payload) > 0:
        result = load_json(next_sync_payload)
        next_sync_payload = ""
    else:
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

@app.route("/_matrix/client/v3/createRoom", methods=["POST"])
def create_room():
    global next_sync_payload
    data = request.get_json()
    if data["name"] != "Super awesome room name" or data["topic"] != "There are not enough raccoons here":
        return dict(), 400
    response = dict()
    response["room_id"] = "!newroom123321:localhost:1234"
    next_sync_payload = "sync_response_new_room"
    return response

@app.route("/_matrix/client/v3/publicRooms", methods=["POST"])
def public_rooms():
    if request.get_json()["filter"]["generic_search_term"] == "forbidden":
        data = dict()
        data["errcode"] = "M_FORBIDDEN"
        data["error"] = "You are not allowed to search for this. Go to https://wikipedia.org for more information"
        return data, 403
    return dict()



if __name__ == "__main__":
    app.run(ssl_context='adhoc', port=1234)
