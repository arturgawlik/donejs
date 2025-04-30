// import doneFetch from "done:fetch.js";
import { HttpParser } from "../lib/http-parser.js";

// `fetch` should get data from some HTTP server
await (async () => {
    const responseToParse = `HTTP/1.1 200 OK
Date: Tue, 29 Apr 2025 17:31:00 GMT
Expires: -1
Cache-Control: private, max-age=0
Content-Type: text/html; charset=ISO-8859-1
Content-Security-Policy-Report-Only: object-src 'none';base-uri 'self';script-src 'nonce--6JAzKm0wIy76Mfik5rcNA' 'strict-dynamic' 'report-sample' 'unsafe-eval' 'unsafe-inline' https: http:;report-uri https://csp.withgoogle.com/csp/gws/other-hp
Server: gws
X-XSS-Protection: 0
X-Frame-Options: SAMEORIGIN
Set-Cookie: AEC=AVcja2cMwA638amB5PznkWxF0P2OoRuId23wUqCPxRowzKcAZz0zgsd1l5E; expires=Sun, 26-Oct-2025 17:31:00 GMT; path=/; domain=.google.com; Secure; HttpOnly; SameSite=lax
Accept-Ranges: none
Vary: Accept-Encoding
Transfer-Encoding: chunked`;

    const parser = new HttpParser('RESPONSE');
    parser.parse(responseToParse)
})();
