"""Offline visual preview. Never sends traffic to the real ESP32."""
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
import json
HTML=(Path(__file__).resolve().parents[1]/'controller_page.h').read_text(encoding='utf-8').split('R"HTML(',1)[1].split(')HTML"',1)[0]
HTML=HTML.replace('Keyboard · mouse · touch','OFFLINE PREVIEW').replace('ESP32 received','Preview received').replace('Ready · all buttons released','Preview · all buttons released')
class Handler(BaseHTTPRequestHandler):
    def log_message(self,*args): pass
    def reply(self,body,kind):
        data=body.encode('utf-8');self.send_response(200);self.send_header('Content-Type',kind+'; charset=utf-8');self.send_header('Content-Length',str(len(data)));self.end_headers();self.wfile.write(data)
    def do_GET(self):
        if self.path=='/status': self.reply(json.dumps(dict(fullReplies=0,dataMismatches=0,rx='PREVIEW',data='PREVIEW')),'application/json')
        else: self.reply(HTML,'text/html')
    def do_POST(self):
        self.rfile.read(int(self.headers.get('Content-Length',0)))
        self.reply('Preview only','text/plain')
if __name__=='__main__':
    print('Offline preview http://127.0.0.1:8788/',flush=True)
    ThreadingHTTPServer(('127.0.0.1',8788),Handler).serve_forever()
