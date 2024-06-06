#!/usr/bin/env python3

import json
from http.server import BaseHTTPRequestHandler, HTTPServer



class SessionRequestHdlr(BaseHTTPRequestHandler):

    def __init__(self, request, client_address, server):
        self.qbits = 3
        super().__init__(request, client_address, server)


    def do_POST(self):
        content_length = int(self.headers['Content-Length'])
        post_data = self.rfile.read(content_length)
        run_def = json.loads(post_data)
        print(f'command={run_def["command"]}')
        res = eval(f'self.cmd_{run_def["command"]}({run_def})')

        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.end_headers()
        self.wfile.write(json.dumps(res).encode("utf-8"))

    def do_GET(self):
        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.end_headers()
        self.wfile.write(json.dumps(self.server.results).encode("utf-8"))

    def cmd_run(self, run_def):
        print(f'settings={run_def["settings"]}')
        self.qbits = len(run_def['init'])
        for gate in run_def['circuit']:
            print(f'    gate={gate}')
        self.server.results = mock_run(self.qbits, run_def['settings']['shots'])
        return ''

    def cmd_circuit(self, run_def):
        print(f'settings={run_def["settings"]}')
        self.qbits = len(run_def['init'])
        for gate in run_def['circuit']:
            print(f'    gate={gate}')
        self.server.results = mock_run(self.qbits, run_def['settings']['shots'])
        return {"id": 1}

    def cmd_get_results(self, run_def):
        return self.server.results


def mock_run(qbits, shots):
    data = []
    for i in range(2**qbits):
        data.append([1,0,i%2])
    return {'type':'results_std', 'data':data}

def main():
    svr = HTTPServer(('', 8000), SessionRequestHdlr)
    svr.results = {'type':'results_inprogress'}

    try:
        svr.serve_forever()
    except KeyboardInterrupt:
        pass

    svr.server_close()



if __name__ == '__main__':
    main()
