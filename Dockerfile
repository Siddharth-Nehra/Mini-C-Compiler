# Stage 1: Build C Compiler and Frontend
FROM node:20-bookworm AS builder

# Install C compiler dependencies
RUN apt-get update && apt-get install -y gcc make && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy full repository
COPY . .

# Build C11 Mini-C Compiler
RUN make clean && make && make test

# Install dependencies and build web frontend
RUN npm --prefix server install
RUN npm --prefix web install
RUN npm --prefix web run build

# Stage 2: Production Execution Environment
FROM node:20-bookworm-slim

RUN apt-get update && apt-get install -y gcc make && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app /app

ENV PORT=3001
ENV NODE_ENV=production

EXPOSE 3001

CMD ["node", "server/src/index.js"]
