import cors from "cors";
import express from "express";
import { access } from "node:fs/promises";
import bcrypt from "bcryptjs";
import jwt from "jsonwebtoken";
import crypto from "node:crypto";
import { compileSource, compilerPath } from "./compile.js";
import { getPrograms, getUsers, savePrograms, saveUsers } from "./db.js";

const PORT = Number(process.env.PORT || 3001);
const JWT_SECRET = process.env.JWT_SECRET || "mini-c-studio-dev-secret";
const TOKEN_DAYS = 7;

const app = express();
app.use(cors({ origin: true, credentials: true }));
app.use(express.json({ limit: "256kb" }));

function signToken(user) {
  return jwt.sign({ sub: user.id, email: user.email, name: user.name }, JWT_SECRET, {
    expiresIn: `${TOKEN_DAYS}d`,
  });
}

function publicUser(user) {
  return {
    id: user.id,
    name: user.name,
    email: user.email,
    createdAt: user.createdAt,
  };
}

function auth(req, res, next) {
  const header = req.headers.authorization || "";
  const token = header.startsWith("Bearer ") ? header.slice(7) : null;
  if (!token) {
    return res.status(401).json({ error: "Please log in to continue" });
  }
  try {
    req.user = jwt.verify(token, JWT_SECRET);
    next();
  } catch {
    return res.status(401).json({ error: "Session expired. Please log in again" });
  }
}

app.get("/api/health", async (_req, res) => {
  let compilerReady = false;
  try {
    await access(compilerPath());
    compilerReady = true;
  } catch {
    compilerReady = false;
  }
  res.json({ ok: true, compilerReady, compiler: compilerPath() });
});

app.post("/api/auth/register", async (req, res) => {
  try {
    const name = String(req.body?.name || "").trim();
    const email = String(req.body?.email || "").trim().toLowerCase();
    const password = String(req.body?.password || "");
    if (name.length < 2) {
      return res.status(400).json({ error: "Name must be at least 2 characters" });
    }
    if (!/^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(email)) {
      return res.status(400).json({ error: "Enter a valid email address" });
    }
    if (password.length < 6) {
      return res.status(400).json({ error: "Password must be at least 6 characters" });
    }
    const users = await getUsers();
    if (users.some((u) => u.email === email)) {
      return res.status(409).json({ error: "An account with that email already exists" });
    }
    const user = {
      id: crypto.randomUUID(),
      name,
      email,
      passwordHash: await bcrypt.hash(password, 10),
      createdAt: new Date().toISOString(),
    };
    users.push(user);
    await saveUsers(users);
    const token = signToken(user);
    res.status(201).json({ token, user: publicUser(user) });
  } catch (err) {
    res.status(500).json({ error: err.message || "Registration failed" });
  }
});

async function ensureDemoUser() {
  try {
    const users = await getUsers();
    const demoEmail = "demo@minic.com";
    if (!users.some((u) => u.email === demoEmail)) {
      users.push({
        id: "demo-user-id-001",
        name: "Demo Student",
        email: demoEmail,
        passwordHash: await bcrypt.hash("password123", 10),
        createdAt: new Date().toISOString(),
      });
      await saveUsers(users);
    }
  } catch (err) {
    console.error("Failed to seed demo user:", err);
  }
}
ensureDemoUser();

async function safeComparePassword(password, hash) {
  if (!password || !hash || typeof hash !== "string") return false;
  try {
    return await bcrypt.compare(password, hash);
  } catch {
    return false;
  }
}

app.post("/api/auth/login", async (req, res) => {
  try {
    const email = String(req.body?.email || "").trim().toLowerCase();
    const password = String(req.body?.password || "");
    if (!email || !password) {
      return res.status(400).json({ error: "Please enter your email and password" });
    }
    const users = await getUsers();
    const user = users.find((u) => u.email === email);
    if (!user || !(await safeComparePassword(password, user.passwordHash))) {
      return res.status(401).json({ error: "Invalid email or password. Please check your credentials or register a new account." });
    }
    res.json({ token: signToken(user), user: publicUser(user) });
  } catch (err) {
    console.error("Login error:", err);
    res.status(401).json({ error: "Invalid email or password. Please check your credentials or register a new account." });
  }
});

app.get("/api/auth/me", auth, async (req, res) => {
  try {
    const users = await getUsers();
    const user = users.find((u) => u.id === req.user.sub);
    if (!user) {
      return res.status(401).json({ error: "Account not found" });
    }
    const programs = (await getPrograms()).filter((p) => p.userId === user.id);
    res.json({ user: publicUser(user), programCount: programs.length });
  } catch (err) {
    res.status(500).json({ error: err.message || "Failed to fetch profile" });
  }
});

app.get("/api/programs", auth, async (req, res) => {
  try {
    const programs = (await getPrograms())
      .filter((p) => p.userId === req.user.sub)
      .sort((a, b) => new Date(b.updatedAt) - new Date(a.updatedAt))
      .map(({ userId: _userId, ...rest }) => rest);
    res.json({ programs });
  } catch (err) {
    res.status(500).json({ error: err.message || "Failed to list programs" });
  }
});

app.get("/api/programs/:id", auth, async (req, res) => {
  try {
    const programs = await getPrograms();
    const program = programs.find((p) => p.id === req.params.id && p.userId === req.user.sub);
    if (!program) {
      return res.status(404).json({ error: "Program not found" });
    }
    const { userId: _userId, ...rest } = program;
    res.json({ program: rest });
  } catch (err) {
    res.status(500).json({ error: err.message || "Failed to fetch program" });
  }
});

app.post("/api/programs", auth, async (req, res) => {
  try {
    const name = String(req.body?.name || "untitled.mc").trim() || "untitled.mc";
    const source = String(req.body?.source ?? STARTER);
    const programs = await getPrograms();
    const program = {
      id: crypto.randomUUID(),
      userId: req.user.sub,
      name: name.endsWith(".mc") ? name : `${name}.mc`,
      source,
      createdAt: new Date().toISOString(),
      updatedAt: new Date().toISOString(),
    };
    programs.push(program);
    await savePrograms(programs);
    const { userId: _userId, ...rest } = program;
    res.status(201).json({ program: rest });
  } catch (err) {
    res.status(500).json({ error: err.message || "Failed to create program" });
  }
});

app.put("/api/programs/:id", auth, async (req, res) => {
  try {
    const programs = await getPrograms();
    const index = programs.findIndex((p) => p.id === req.params.id && p.userId === req.user.sub);
    if (index === -1) {
      return res.status(404).json({ error: "Program not found" });
    }
    if (typeof req.body?.name === "string" && req.body.name.trim()) {
      const name = req.body.name.trim();
      programs[index].name = name.endsWith(".mc") ? name : `${name}.mc`;
    }
    if (typeof req.body?.source === "string") {
      programs[index].source = req.body.source;
    }
    programs[index].updatedAt = new Date().toISOString();
    await savePrograms(programs);
    const { userId: _userId, ...rest } = programs[index];
    res.json({ program: rest });
  } catch (err) {
    res.status(500).json({ error: err.message || "Failed to update program" });
  }
});

app.delete("/api/programs/:id", auth, async (req, res) => {
  try {
    const programs = await getPrograms();
    const next = programs.filter((p) => !(p.id === req.params.id && p.userId === req.user.sub));
    if (next.length === programs.length) {
      return res.status(404).json({ error: "Program not found" });
    }
    await savePrograms(next);
    res.json({ ok: true });
  } catch (err) {
    res.status(500).json({ error: err.message || "Failed to delete program" });
  }
});

app.post("/api/compile", auth, async (req, res) => {
  try {
    const result = await compileSource(req.body?.source ?? "");
    res.json(result);
  } catch (err) {
    res.status(err.status || 500).json({ error: err.message || "Compile failed" });
  }
});

const STARTER = `int a = 10;
int b = 20;
int c;

c = a + b;

print(c);
`;

app.listen(PORT, () => {
  console.log(`Mini-C API listening on http://localhost:${PORT}`);
});
