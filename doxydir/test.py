#!/usr/bin/env python3

# Copyright (c) 2023 Fraunhofer IIS
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the “Software”), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.


import errno
import json
import socket
import select

def process_request(cmd_request,sock):
	json_cmd = json.dumps(cmd_request) + "\n"
	sock.sendall(json_cmd.encode())

	resp = sock.recv(4096)

	# Just dump if not empty
	if resp:
		print(json.dumps(json.loads(resp.decode("utf-8")), indent=4))

def main():
	IP = 'localhost'
	TCP_PORT = 1234
	counter = 0 # response counter

	try:
		sock = socket.create_connection((IP, TCP_PORT))
	except socket.error as e:
		if e.errno != errno.ECONNREFUSED:
			print("Exception was thrown. Message is %s" % (e))
			return 1
		print("A connection to '%s:%d' could not be established." %(IP, TCP_PORT))
		return 1
		
	cmd_request = {
	"japi_request": "get_temperature",
	"args": {"unit": "kelvin"},
	}
	push_service_request = {
	"japi_request": "japi_pushsrv_list",
	}
	push_temperature_request_subscr = {
	"japi_request": "japi_pushsrv_subscribe",
	"args": {"service": "push_temperature"},
	}
	push_temperature_request_unsubscr = {
	"japi_request": "japi_pushsrv_unsubscribe",
	"args": {"service": "push_temperature"},
	}

	process_request(cmd_request,sock)
	process_request(push_service_request,sock)
	process_request(push_temperature_request_subscr,sock)

	# get push service messages
	sock.settimeout(10)
	try:
		resp = sock.recv(4096)
		# Iterate line by line
		for n, line in enumerate(sock.makefile(), start=1):
			# Just dump if received message is not empty
			if line:
				jdata = json.loads(line)
				print(json.dumps(json.loads(line), indent=4))
	except:
		pass
	finally:
		pass

	process_request(push_temperature_request_unsubscr,sock)

	sock.close()

if __name__ == "__main__":
	main()
