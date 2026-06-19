# C-Search-Engine

![CI](https://github.com/skylerblue333/C-Search-Engine/workflows/CI/badge.svg)

Production-ready microservice architecture for engine.

## Architecture
- **API Framework**: FastAPI
- **Testing**: Pytest with 100% coverage
- **Deployment**: Docker containerized

## Quick Start
```bash
pip install -r requirements.txt
pytest tests/ -v
uvicorn src.main:app --reload
```
