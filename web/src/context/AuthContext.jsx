import { createContext, useContext, useEffect, useMemo, useState } from "react";
import { fetchMe, login as apiLogin, register as apiRegister } from "../api/services";

const AuthContext = createContext(null);

export function AuthProvider({ children }) {
  const [token, setToken] = useState(() => localStorage.getItem("minic_token"));
  const [user, setUser] = useState(null);
  const [loading, setLoading] = useState(Boolean(token));

  useEffect(() => {
    if (!token) {
      setUser(null);
      setLoading(false);
      return;
    }
    fetchMe()
      .then((data) => setUser(data.user))
      .catch(() => {
        localStorage.removeItem("minic_token");
        setToken(null);
        setUser(null);
      })
      .finally(() => setLoading(false));
  }, [token]);

  const value = useMemo(
    () => ({
      user,
      token,
      loading,
      async login(email, password) {
        const data = await apiLogin(email, password);
        localStorage.setItem("minic_token", data.token);
        setToken(data.token);
        setUser(data.user);
        return data.user;
      },
      async register(name, email, password) {
        const data = await apiRegister(name, email, password);
        localStorage.setItem("minic_token", data.token);
        setToken(data.token);
        setUser(data.user);
        return data.user;
      },
      logout() {
        localStorage.removeItem("minic_token");
        setToken(null);
        setUser(null);
      },
    }),
    [user, token, loading]
  );

  return <AuthContext.Provider value={value}>{children}</AuthContext.Provider>;
}

export function useAuth() {
  const ctx = useContext(AuthContext);
  if (!ctx) {
    throw new Error("useAuth must be used within AuthProvider");
  }
  return ctx;
}
