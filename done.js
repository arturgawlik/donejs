import doneConsole from "done:console.js";
import doneProcess from "done:process.js";
import doneTextDecoder from "done:text-decoder.js";
import doneFetch from "done:fetch.js";

globalThis.console = doneConsole;
globalThis.process = doneProcess;
globalThis.exit = doneProcess.exit;
globalThis.TextDecoder = doneTextDecoder
globalThis.fetch = doneFetch;