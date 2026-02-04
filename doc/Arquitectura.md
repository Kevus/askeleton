ASkeleTon - Arquitectura (resumen)

Flujo general
1. Clang Tooling analiza el AST usando `compile_commands.json`.
2. `ASKGen` detecta funciones, metodos y constructores.
3. `Generator` (Boost/Catch/GTest) crea los archivos de test y fixture.
4. `ConfigGenerator` crea el `.cfg` con datos de entrada/salida.
5. `RandomValuesGenerator` rellena datos aleatorios o con perfil.
6. Reglas `--rule-data` inyectan valores derivados del AST cuando aplica.

Componentes principales
- `src/askeleton.cpp`: CLI y configuracion global.
- `src/ASKGen.cpp`: recoleccion de funciones y reglas del AST.
- `src/framework/*Gen.cpp`: generadores de test por framework.
- `src/ConfigGenerator.cpp`: escritura de `.cfg`.
- `src/RandomValuesGenerator.cpp`: generacion de datos (random, boundary, safe, stress).

Puntos de extension
- Nuevas reglas: `ASKGen::collectRuleValuesFromFunction`.
- Nuevos perfiles: `RandomValuesGenerator::setProfile`.
- Nuevos formatos de salida: clases en `src/framework/`.
