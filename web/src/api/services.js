import client from "./client";
import { ENDPOINTS } from "./config";

export function login(email, password) {
  return client.post(ENDPOINTS.login, { email, password }).then((r) => r.data);
}

export function register(name, email, password) {
  return client.post(ENDPOINTS.register, { name, email, password }).then((r) => r.data);
}

export function fetchMe() {
  return client.get(ENDPOINTS.me).then((r) => r.data);
}

export function fetchHealth() {
  return client.get(ENDPOINTS.health).then((r) => r.data);
}

export function listPrograms() {
  return client.get(ENDPOINTS.programs).then((r) => r.data.programs);
}

export function getProgram(id) {
  return client.get(ENDPOINTS.program(id)).then((r) => r.data.program);
}

export function createProgram(payload) {
  return client.post(ENDPOINTS.programs, payload).then((r) => r.data.program);
}

export function updateProgram(id, payload) {
  return client.put(ENDPOINTS.program(id), payload).then((r) => r.data.program);
}

export function deleteProgram(id) {
  return client.delete(ENDPOINTS.program(id)).then((r) => r.data);
}

export function compileSource(source) {
  return client.post(ENDPOINTS.compile, { source }).then((r) => r.data);
}
