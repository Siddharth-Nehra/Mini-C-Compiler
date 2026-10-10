import axios from "axios";
import { API_BASE } from "./config";

const client = axios.create({
  baseURL: API_BASE,
  timeout: 20000,
});

client.interceptors.request.use((config) => {
  const token = localStorage.getItem("minic_token");
  if (token) {
    config.headers.Authorization = `Bearer ${token}`;
  }
  return config;
});

client.interceptors.response.use(
  (res) => res,
  (err) => {
    let message = "An error occurred";
    if (err.response?.data?.error) {
      message = err.response.data.error;
    } else if (err.response?.data?.message) {
      message = err.response.data.message;
    } else if (typeof err.response?.data === "string" && err.response.data.trim()) {
      message = err.response.data.includes("<!DOCTYPE") || err.response.data.includes("<html")
        ? "Server internal error. Please check server logs."
        : err.response.data;
    } else if (err.code === "ERR_NETWORK" || !err.response) {
      message = "Cannot connect to Mini-C API server. Ensure the backend server is running on http://localhost:3001.";
    } else if (err.message) {
      message = err.message;
    }
    return Promise.reject(new Error(message));
  }
);

export default client;
