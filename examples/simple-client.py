# This program demonstrates how we can use the API.
# 
# Usage: python3 simple-client.py "artist name" "song title"

import argparse
import json
import requests

# Default API URI format
HOST     = "localhost"
PORT     = 8080
ENDPOINT = "api/lyrics"

# Status codes used by API
HTTP_SUCCESS      = 200
HTTP_BAD_REQUEST  = 400
HTTP_NOT_FOUND    = 404
HTTP_SERVER_ERROR = 500


def main():
    args = get_args()
    try:
        res = make_request(args)
    except Exception:
        print("Couldn't contact server!")
        return
    handle_response(res, args)

def get_args():
    parser = argparse.ArgumentParser(formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("artist", help=(
        "Artist Name\n"
        "Example: name\n"
        "Example: \"multi-word name\""
    ))
    parser.add_argument("song", help=(
        "Song Title\n"
        "Example: title\n"
        "Example: \"multi-word title\""
    ))
    args = parser.parse_args()

    return args

def make_request(args):
    return requests.get(f"http://{HOST}:{PORT}/{ENDPOINT}/{args.artist}/{args.song}/")

def handle_response(res, args):
    err_code = get_errcode(res)
    if err_code != HTTP_SUCCESS:
        if err_code == HTTP_BAD_REQUEST:
            print(f"{get_errmsg(res)}")
        elif err_code == HTTP_NOT_FOUND:
            print(f"{args.song} by {args.artist} not found")
        elif err_code == HTTP_SERVER_ERROR:
            print("Something went wrong!")
        return

    print('\n' + '-' * 20 + f" {get_song(res)} by {get_artist(res)} " + '-' * 20 + '\n' + get_lyrics(res))

def get_errcode(res):
    return res.json()["error"]["code"]

def get_errmsg(res):
    return res.json()["error"]["message"]

def get_artist(res):
    return res.json()["data"]["artist"]

def get_song(res):
    return res.json()["data"]["song"]

def get_lyrics(res):
    return res.json()["data"]["lyrics"]


if __name__ == "__main__":
    main()
