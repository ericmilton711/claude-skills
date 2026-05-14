# Tandoor Recipes — Self-Hosted Recipe Manager

## Overview
Tandoor Recipes is a self-hosted recipe management app running in Docker on the ThinkCentre (192.168.12.136). It provides recipe storage, meal planning, and shopping list generation accessible from any device on the MILTONHAUS network.

## Access — ThinkCentre Instance
- **URL:** http://192.168.12.136:8090
- **Username:** miltonhaus
- **Password:** 645866
- **Port:** 8090 (mapped to container port 80; port 80 was taken by Pi-hole)
- **Network:** MILTONHAUS LAN only (no external access unless reverse proxy or WireGuard is configured)

## Access — Lambert Instance
- **URL:** http://192.168.0.104:8380 (via WireGuard Lambert tunnel)
- **Credentials:** Unknown — need to get from the Lamberts
- **Note:** SSH to 192.168.0.104 is closed; no admin access to this instance

## Server Location
- **Host:** ThinkCentre (192.168.12.136), user `milton`
- **Install directory:** ~/tandoor/
- **Docker Compose file:** ~/tandoor/docker-compose.yml
- **Environment file:** ~/tandoor/.env

## Docker Containers
- `tandoor-web_recipes-1` — Tandoor web app (vabene1111/recipes)
- `tandoor-db_recipes-1` — PostgreSQL 16 Alpine database

## Docker Compose Configuration
```yaml
services:
  db_recipes:
    restart: always
    image: postgres:16-alpine
    volumes:
      - ./postgresql:/var/lib/postgresql/data
    env_file:
      - ./.env

  web_recipes:
    restart: always
    image: vabene1111/recipes
    env_file:
      - ./.env
    ports:
      - 8090:80
    volumes:
      - staticfiles:/opt/recipes/staticfiles
      - ./mediafiles:/opt/recipes/mediafiles
    depends_on:
      - db_recipes

volumes:
  staticfiles:
```

## Environment Variables (.env)
```
SECRET_KEY=NUnunNSc1o1DnFMlATvThlULGkCl5pM6inWZ+1n57ZQ6qX7Aom
TZ=America/New_York
ALLOWED_HOSTS=*
DB_ENGINE=django.db.backends.postgresql
POSTGRES_HOST=db_recipes
POSTGRES_DB=djangodb
POSTGRES_PORT=5432
POSTGRES_USER=djangouser
POSTGRES_PASSWORD=vSpWVJcPzLn39BJqa7Wl
```

## Data Directories
- **PostgreSQL data:** ~/tandoor/postgresql/
- **Media files (recipe images, etc.):** ~/tandoor/mediafiles/
- **Static files:** Docker volume `tandoor_staticfiles`

## SELinux Note (Fedora)
The ThinkCentre runs Fedora with SELinux enforcing. The bind-mount directories required the `container_file_t` SELinux context:
```bash
sudo chcon -Rt container_file_t ~/tandoor/postgresql ~/tandoor/mediafiles
sudo chown -R 999:999 ~/tandoor/postgresql
```
Without this, the PostgreSQL container will crash-loop with permission denied errors.

## Management Commands
```bash
# SSH to ThinkCentre
ssh -i ~/.ssh/id_ed25519 -o StrictHostKeyChecking=no milton@192.168.12.136

# Check status
cd ~/tandoor && docker compose ps

# View logs
docker logs tandoor-web_recipes-1
docker logs tandoor-db_recipes-1

# Restart
cd ~/tandoor && docker compose restart

# Stop
cd ~/tandoor && docker compose down

# Start
cd ~/tandoor && docker compose up -d

# Update to latest version
cd ~/tandoor && docker compose pull && docker compose up -d
```

## Features
- **Import from URL:** Paste a recipe website link and Tandoor auto-imports ingredients, steps, and photos
- **Manual entry:** Type in family recipes, handwritten recipes, etc.
- **Photo-to-recipe workflow:** Take a picture of a recipe page (cookbook, card) and have Claude read the image and extract the text, then paste into Tandoor
- **Meal planning:** Plan weekly meals and auto-generate shopping lists
- **Sharing:** Create user accounts for family, or generate share links for individual recipes (no login needed)
- **Multi-user:** Multiple family members can have their own accounts

## Docs
- Official documentation: https://docs.tandoor.dev/
- Docker install guide: https://docs.tandoor.dev/install/docker/

## Install Date
2026-05-14
