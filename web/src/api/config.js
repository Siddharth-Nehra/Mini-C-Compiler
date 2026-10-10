export const API_BASE = import.meta.env.VITE_API_BASE_URL || "http://localhost:3001/api";

export const ENDPOINTS = {
  health: "/health",
  login: "/auth/login",
  register: "/auth/register",
  me: "/auth/me",
  programs: "/programs",
  program: (id) => `/programs/${id}`,
  compile: "/compile",
};
