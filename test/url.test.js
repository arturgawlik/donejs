import assert from "done:assert.js";
import URL from "done:url.js";

// `URL` should properly parse valid url string
(() => {
    const urlToParse = 'http://www.google.com:80/some/path-to-test?with=query'

    const url = new URL(urlToParse);

    // testing only those properties that are needed at the moment
    assert.equal(url.href, urlToParse, 'url.href should be exactly input url string');
    assert.equal(url.hostname, "www.google.com");
    assert.equal(url.protocol, "http:");
    assert.equal(url.port, "80");
})();
