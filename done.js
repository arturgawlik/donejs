import doneConsole from "done:console.js";
import doneProcess from "done:process.js";
import doneTextDecoder from "done:text-decoder.js";
import doneFetch from "done:fetch.js";
import doneSyscallWrapper from "done:syscall-wrapper.js";
import doneInterpretFlags from "done:argv.js";

import "done:special-mode-dispatcher.js";

globalThis.console = doneConsole;
globalThis.process = doneProcess;
globalThis.exit = doneProcess.exit;
globalThis.TextDecoder = doneTextDecoder
globalThis.fetch = doneFetch;

// TODO: maybe make done obj (and other build in?) stuff not overridable?
globalThis.done.interpretFlags = doneInterpretFlags;
globalThis.done.syscall = doneSyscallWrapper;
