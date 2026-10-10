import { mkdir, readFile, writeFile } from "node:fs/promises";
import path from "node:path";
import { fileURLToPath } from "node:url";

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const DATA_DIR = path.resolve(__dirname, "../data");
const USERS_FILE = path.join(DATA_DIR, "users.json");
const PROGRAMS_FILE = path.join(DATA_DIR, "programs.json");

let queue = Promise.resolve();

async function readJson(file, fallback) {
  try {
    const raw = await readFile(file, "utf8");
    return JSON.parse(raw);
  } catch {
    return fallback;
  }
}

function writeJson(file, data) {
  queue = queue.then(async () => {
    await mkdir(DATA_DIR, { recursive: true });
    await writeFile(file, JSON.stringify(data, null, 2));
  });
  return queue;
}

export async function getUsers() {
  return readJson(USERS_FILE, []);
}

export function saveUsers(users) {
  return writeJson(USERS_FILE, users);
}

export async function getPrograms() {
  return readJson(PROGRAMS_FILE, []);
}

export function savePrograms(programs) {
  return writeJson(PROGRAMS_FILE, programs);
}
