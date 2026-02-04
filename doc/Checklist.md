Checklist de verificacion (release)

Fecha: 2026-02-04

Comandos recomendados
1) Basico
ASKELETON_HOME=$(pwd) ./askeleton -p examples examples/sut.cpp

2) Regla de datos
ASKELETON_HOME=$(pwd) ./askeleton --rule-data --rule-max-cases=3 -p examples examples/sut.cpp

3) Perfiles
ASKELETON_HOME=$(pwd) ./askeleton --profile=random -p examples examples/sut.cpp
ASKELETON_HOME=$(pwd) ./askeleton --profile=boundary -p examples examples/sut.cpp
ASKELETON_HOME=$(pwd) ./askeleton --profile=safe -p examples examples/sut.cpp
ASKELETON_HOME=$(pwd) ./askeleton --profile=stress -p examples examples/sut.cpp

4) Determinismo
ASKELETON_HOME=$(pwd) ./askeleton --seed=123 -p examples examples/sut.cpp

Automatizado:
scripts/check_all.sh

Resultados esperados
- Se generan archivos en Generated/UT/<target>/
- El cfg contiene valores no vacios en perfiles random/safe.
- Boundary puede incluir contenedores vacios por diseño.
- Regla de datos genera multiples casos para comparaciones.
