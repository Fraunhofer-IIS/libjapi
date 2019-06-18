#!/usr/bin/env python3

import json
import socket
import select

def process_request(cmd_request,sock):
	json_cmd = json.dumps(cmd_request) + "\n"
	sock.sendall(json_cmd.encode())

	resp = sock.recv(4096)

	print(json.dumps(json.loads(resp.decode("utf-8")), indent=4))

def main():
	IP = 'localhost'
	TCP_PORT = 1234
	counter = 0 # response counter

	try:
		sock = socket.create_connection((IP, TCP_PORT))
	except (ConnectionError):
		log.error("A connection to %s:%s could not be established.", *addr)
		return {}

	cmd_request = {
	"japi_request": "get_temperature",
	"unit": "kelvin",
	}
	push_service_request = {
	"japi_request": "japi_pushsrv_list",
	}
	push_temperature_request_subscr = {
	"japi_request": "japi_pushsrv_subscribe",
	"service": "push_temperature",
	}
	push_temperature_request_unsubscr = {
	"japi_request": "japi_pushsrv_unsubscribe",
	"service": "push_temperature",
	}
	push_counter_request_subscr = {
	"japi_request": "japi_pushsrv_subscribe",
	"service": "push_counter",
	}
	push_counter_request_unsubscr = {
	"japi_request": "japi_pushsrv_unsubscribe",
	"service": "push_counter",
	}

	process_request(cmd_request,sock)
	process_request(push_service_request,sock)
	process_request(push_temperature_request_subscr,sock)
	process_request(push_counter_request_subscr,sock)

	# get push service messages
	sock.settimeout(10)
	try:
		resp = sock.recv(4096)
		# Iterate line by line
		for n, line in enumerate(sock.makefile(), start=1):
			jdata = json.loads(line)
			print(json.dumps(json.loads(line), indent=4))
	except:
		pass
	finally:
		pass

	process_request(push_temperature_request_unsubscr,sock)
	process_request(push_counter_request_unsubscr,sock)

	sock.close()

if __name__ == "__main__":
	main()
