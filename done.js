import doneConsole from "done:console.js";
import doneProcess from "done:process.js";
import doneTextDecoder from "done:text-decoder.js";
import doneTextEncoder from "done:text-encoder.js";
import doneFetch from "done:fetch.js";
import doneURL from "done:url.js";
import doneSyscallWrapper from "done:syscall-wrapper.js";
import doneInterpretFlags from "done:argv.js";

import "done:special-mode-dispatcher.js";

globalThis.console = doneConsole;
globalThis.process = doneProcess;
globalThis.exit = doneProcess.exit;
globalThis.TextDecoder = doneTextDecoder
globalThis.TextEncoder = doneTextEncoder
globalThis.fetch = doneFetch;
globalThis.URL = doneURL;

// TODO: maybe make done obj (and other build in?) stuff not overridable?
globalThis.done.interpretFlags = doneInterpretFlags;
globalThis.done.syscall = doneSyscallWrapper;
