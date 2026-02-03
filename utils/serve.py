#!/usr/bin/env python3

from http.server import HTTPServer, SimpleHTTPRequestHandler

class COEPHandler(SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        super().end_headers()

HTTPServer(("localhost", 8000), COEPHandler).serve_forever()
