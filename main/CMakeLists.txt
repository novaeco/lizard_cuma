
idf_component_register(
  SRCS "main.c"              # ou main.cpp, etc.
  INCLUDE_DIRS "."           # ou dossiers d’headers
  REQUIRES lovyangfx lvgl  esp_wifi  # liste des dépendances IDF_COMPONENTS
)