/**
 * URL parser based on https://github.com/unshiftio/url-parse/tree/0.1 implementation.
 */

class URLParserError extends Error {
    constructor(msg) {
        super(msg);
    }
}

const regexp = /^(?:(?:(([^:\/#\?]+:)?(?:(?:\/\/)(?:(?:(?:([^:@\/#\?]+)(?:\:([^:@\/#\?]*))?)@)?(([^:\/#\?\]\[]+|\[[^\/\]@#?]+\])(?:\:([0-9]+))?))?)?)?((?:\/?(?:[^\/\?#]+\/+)*)(?:[^\?#]*)))?(\?[^#]+)?)(#.*)?/
    , keys = 'href,,protocol,username,password,host,hostname,port,pathname,query,hash'.split(',')
    , parts = keys.length;

export default class URL {
    /**
     * @param {string | URL} url 
     * @param {string | URL | undefined} base
     */
    constructor(url, base) {
        if (typeof url !== 'string') {
            throw new URLParserError(`"url" argument must be string. Got: "${typeof url}."`);
        }
        if (base) {
            throw new URLParserError('"base" is not currently supported.')
        }

        this.#parse(url, base);
    }

    /**
     * @param {string | URL} url 
     * @param {string | URL | undefined} base
     */
    #parse(url, base) {
        for (let i = 0, bits = regexp.exec(url), key = keys[i]; i < parts; key = keys[++i]) {
            if (key) {
                this[key] = bits[i];
            }
        }
    }
}
