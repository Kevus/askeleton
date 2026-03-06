#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ASKELETON_BIN="${ROOT_DIR}/askeleton"
export ASKELETON_HOME="${ASKELETON_HOME:-$ROOT_DIR}"

MODE="quick"
SUT_KIND="showcase"
OUT_DIR="${ROOT_DIR}/demo_runs/$(date +%Y%m%d_%H%M%S)"
BUILD_PATH="${ROOT_DIR}/examples"

usage() {
  cat <<'USAGE'
Uso:
  ./scripts/demo_askeleton.sh [--quick|--full] [--sut showcase|basic] [--out DIR]

Opciones:
  --quick            Demo guiada corta [por defecto]
  --full             Demo guiada completa
  --sut showcase     Usa examples/sut_showcase.cpp [por defecto]
  --sut basic        Usa examples/sut.cpp
  --out DIR          Carpeta base de salida
  --help             Muestra esta ayuda

Controles durante la demo:
  Enter              Avanzar al siguiente subpaso
  q                  Salir
USAGE
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --quick)
      MODE="quick"
      shift
      ;;
    --full)
      MODE="full"
      shift
      ;;
    --sut)
      [[ $# -ge 2 ]] || { echo "ERROR: falta valor para --sut"; exit 1; }
      SUT_KIND="$2"
      shift 2
      ;;
    --out)
      [[ $# -ge 2 ]] || { echo "ERROR: falta valor para --out"; exit 1; }
      OUT_DIR="$2"
      shift 2
      ;;
    --help|-h)
      usage
      exit 0
      ;;
    *)
      echo "ERROR: opción no reconocida: $1"
      usage
      exit 1
      ;;
  esac
done

[[ -x "$ASKELETON_BIN" ]] || { echo "ERROR: no encuentro ejecutable en $ASKELETON_BIN"; exit 1; }

if [[ "$SUT_KIND" == "showcase" ]]; then
  SUT_FILE="${ROOT_DIR}/examples/sut_showcase.cpp"
elif [[ "$SUT_KIND" == "basic" ]]; then
  SUT_FILE="${ROOT_DIR}/examples/sut.cpp"
else
  echo "ERROR: --sut debe ser 'showcase' o 'basic'"
  exit 1
fi
[[ -f "$SUT_FILE" ]] || { echo "ERROR: no encuentro $SUT_FILE"; exit 1; }

mkdir -p "$OUT_DIR"

pause_or_quit() {
  local answer
  read -r -p "Pulsa Enter para continuar (q para salir): " answer
  if [[ "$answer" == "q" || "$answer" == "Q" ]]; then
    echo "Demo interrumpida por el usuario."
    exit 0
  fi
}

print_sep() {
  echo
  echo "================================================================"
}

run_best_effort() {
  local cmd="$1"
  set +e
  eval "$cmd"
  local status=$?
  set -e
  if [[ $status -ne 0 ]]; then
    echo "[WARN] Comando terminó con código $status"
  fi
}

show_near() {
  local file="$1"
  local pattern="$2"
  local before="${3:-3}"
  local after="${4:-18}"
  local line
  line="$(rg -n --fixed-strings "$pattern" "$file" | head -n 1 | cut -d: -f1 || true)"
  if [[ -z "$line" ]]; then
    echo "[WARN] No se encontró patrón: $pattern"
    return 0
  fi
  local start=$(( line - before ))
  local end=$(( line + after ))
  (( start < 1 )) && start=1
  nl -ba "$file" | sed -n "${start},${end}p"
}

run_guided_step() {
  local id="$1"
  local title="$2"
  local context_msg="$3"
  local show_code_cmd="$4"
  local run_cmd="$5"
  local show_artifacts_cmd="$6"

  print_sep
  echo "[$id] $title"
  echo "$context_msg"

  echo
  echo "Subpaso 1/4: contexto"
  pause_or_quit
  echo "$context_msg"

  echo
  echo "Subpaso 2/4: veamos este código"
  pause_or_quit
  run_best_effort "$show_code_cmd"

  echo
  echo "Subpaso 3/4: generemos pruebas"
  pause_or_quit
  echo "Comando: $run_cmd"
  run_best_effort "$run_cmd"

  echo
  echo "Subpaso 4/4: veamos qué se generó"
  pause_or_quit
  run_best_effort "$show_artifacts_cmd"

  echo
  echo "Paso [$id] completado."
}

show_header() {
  echo "Demo interactiva de ASkeleTon"
  echo "Modo: $MODE"
  echo "SUT: $SUT_FILE"
  echo "Salida base: $OUT_DIR"
  echo "ASKELETON_HOME: $ASKELETON_HOME"
}

step_intro_only() {
  print_sep
  echo "[00] Intro"
  echo "Subpaso 1/2: objetivo"
  pause_or_quit
  echo "Objetivo: mostrar código fuente, ejecutar ASkeleTon y revisar artefactos en cada parada."

  echo
  echo "Subpaso 2/2: mapa del showcase"
  pause_or_quit
  run_best_effort "nl -ba '$SUT_FILE' | sed -n '1,120p'"

  echo
  echo "Paso [00] completado."
}

run_quick_showcase() {
  run_guided_step \
    "01" \
    "Funciones Base" \
    "Primero vemos funciones libres y las pruebas que se generan para esas funciones." \
    "show_near '$SUT_FILE' 'int add_i64' 2 12; echo; show_near '$SUT_FILE' 'int classify_threshold' 2 14" \
    "ASKELETON_HOME='$ASKELETON_HOME' '$ASKELETON_BIN' -p '$BUILD_PATH' --out-dir '$OUT_DIR/01_functions' '$SUT_FILE'" \
    "echo '[Target funcion libre: sut_showcase]'; find '$OUT_DIR/01_functions/sut_showcase' -maxdepth 1 -type f | sort; echo; echo '[Test add_i64]'; sed -n '/TEST_F(Fixture, sut_showcase_add_i64_1)/,/^}/p' '$OUT_DIR/01_functions/sut_showcase/sut_showcase_test.cpp'; echo; echo '[CFG add_i64 + classify_threshold]'; sed -n '/add_i64_1:/,/sum_ptrs_1:/p' '$OUT_DIR/01_functions/sut_showcase/sut_showcase.cfg'"

  run_guided_step \
    "02" \
    "Rule-Based Data" \
    "Comparaciones del código alimentan datos de prueba." \
    "show_near '$SUT_FILE' 'int classify_threshold' 2 14" \
    "ASKELETON_HOME='$ASKELETON_HOME' '$ASKELETON_BIN' -p '$BUILD_PATH' --rule-data --rule-max-cases=5 --out-dir '$OUT_DIR/02_rule_data' '$SUT_FILE'" \
    "echo '[CFG classify_threshold]'; rg -n '^classify_threshold_[0-9]+:' '$OUT_DIR/02_rule_data/sut_showcase/sut_showcase.cfg' || true; echo; sed -n '/classify_threshold_1:/,/sum_ptrs_1:/p' '$OUT_DIR/02_rule_data/sut_showcase/sut_showcase.cfg'"

  run_guided_step \
    "03" \
    "Tipos Estructurados" \
    "optional/pair/tuple y su reflejo en .cfg." \
    "show_near '$SUT_FILE' 'std::optional<int> clamp_optional' 2 12; echo; show_near '$SUT_FILE' 'std::pair<int, int> split_sign' 2 10; echo; show_near '$SUT_FILE' 'std::tuple<int, short, long long> widen_tuple' 2 8" \
    "ASKELETON_HOME='$ASKELETON_HOME' '$ASKELETON_BIN' -p '$BUILD_PATH' --out-dir '$OUT_DIR/03_structured' '$SUT_FILE'" \
    "echo '[CFG structured keys]'; rg -n 'has_value|\\.value|\\.first|\\.second|\\.0=|\\.1=|\\.2=' '$OUT_DIR/03_structured/sut_showcase/sut_showcase.cfg' || true; echo; sed -n '/clamp_optional_1:/,/add_and_store_1:/p' '$OUT_DIR/03_structured/sut_showcase/sut_showcase.cfg'"

  run_guided_step \
    "04" \
    "Clases y Metodos" \
    "Ahora sí: mostramos clases del SUT y sus pruebas generadas (constructores, metodos, static)." \
    "show_near '$SUT_FILE' 'class Counter' 2 24; echo; show_near '$SUT_FILE' 'class NonDefaultAccumulator' 2 14" \
    "ASKELETON_HOME='$ASKELETON_HOME' '$ASKELETON_BIN' -p '$BUILD_PATH' --out-dir '$OUT_DIR/04_classes' '$SUT_FILE'" \
    "echo '[Targets de clase]'; find '$OUT_DIR/04_classes' -mindepth 1 -maxdepth 1 -type d | rg 'Counter|NonDefaultAccumulator' || true; echo; echo '[Counter test]'; sed -n '1,160p' '$OUT_DIR/04_classes/Counter/Counter_test.cpp'; echo; echo '[Counter cfg]'; sed -n '1,120p' '$OUT_DIR/04_classes/Counter/Counter.cfg'"

  run_guided_step \
    "05" \
    "Skips En Strict" \
    "En modo strict se omiten mutables y construcciones no default para ser conservador." \
    "show_near '$SUT_FILE' 'class NonDefaultAccumulator' 2 18; echo; show_near '$SUT_FILE' 'int sum_ptrs' 2 10; echo; show_near '$SUT_FILE' 'int add_and_store' 2 10" \
    "ASKELETON_HOME='$ASKELETON_HOME' '$ASKELETON_BIN' -p '$BUILD_PATH' --coverage-mode=strict --report='$OUT_DIR/05_strict_report.json' --out-dir '$OUT_DIR/05_strict' '$SUT_FILE'" \
    "echo '[Resumen strict]'; rg -n 'coverage_mode|\"skipped\"|coverage_policy_mutable_parameter|coverage_policy_instance_construction|\"by_reason\"' '$OUT_DIR/05_strict_report.json' || true"

  run_guided_step \
    "06" \
    "Skips Estructurales" \
    "Casos límite del SUT que se reportan explícitamente como no soportados." \
    "show_near '$SUT_FILE' 'int apply_processor' 3 8; echo; show_near '$SUT_FILE' 'int count_c_strings' 2 12; echo; show_near '$SUT_FILE' 'int sum_matrix_2x2' 2 8" \
    "ASKELETON_HOME='$ASKELETON_HOME' '$ASKELETON_BIN' -p '$BUILD_PATH' --coverage-mode=balanced --report='$OUT_DIR/06_balanced_report.json' --out-dir '$OUT_DIR/06_balanced' '$SUT_FILE'" \
    "echo '[Resumen balanced]'; rg -n '\"reason\"|abstract_record|unsupported_indirection|unsupported_type_shape|\"by_reason\"|\"skipped\"' '$OUT_DIR/06_balanced_report.json' || true"

  run_guided_step \
    "07" \
    "Framework Alternativo" \
    "Mismo caso de clase, salida para Boost.Test." \
    "show_near '$SUT_FILE' 'class Counter' 2 24" \
    "ASKELETON_HOME='$ASKELETON_HOME' '$ASKELETON_BIN' -p '$BUILD_PATH' --framework=boost --out-dir '$OUT_DIR/07_boost' '$SUT_FILE'" \
    "echo '[Arbol boost]'; find '$OUT_DIR/07_boost' -mindepth 1 -maxdepth 1 -type d | sort; echo; echo '[Boost clase Counter - test]'; sed -n '1,140p' '$OUT_DIR/07_boost/Counter/Counter_test.cpp'; echo; echo '[Boost clase Counter - cfg]'; sed -n '1,120p' '$OUT_DIR/07_boost/Counter/Counter.cfg'"

  run_guided_step \
    "08" \
    "Oracle Mode" \
    "Comparación final: explicit vs property sobre la misma función." \
    "show_near '$SUT_FILE' 'std::vector<int> scale_vec' 2 12" \
    "ASKELETON_HOME='$ASKELETON_HOME' '$ASKELETON_BIN' -p '$BUILD_PATH' --oracle-mode=explicit --out-dir '$OUT_DIR/08_oracle_explicit' '$SUT_FILE' && ASKELETON_HOME='$ASKELETON_HOME' '$ASKELETON_BIN' -p '$BUILD_PATH' --oracle-mode=property --out-dir '$OUT_DIR/08_oracle_property' '$SUT_FILE'" \
    "echo '[Explicit - bloque scale_vec]'; sed -n '/TEST_F(Fixture, sut_showcase_scale_vec_1)/,/^}/p' '$OUT_DIR/08_oracle_explicit/sut_showcase/sut_showcase_test.cpp'; echo; echo '[Property - bloque scale_vec]'; sed -n '/TEST_F(Fixture, sut_showcase_scale_vec_1)/,/^}/p' '$OUT_DIR/08_oracle_property/sut_showcase/sut_showcase_test.cpp'"
}

run_full_extras_showcase() {
  run_guided_step \
    "08" \
    "Perfiles De Datos" \
    "Comparamos boundary/safe/stress con seed fija." \
    "show_near '$SUT_FILE' 'int classify_threshold' 2 12; echo; show_near '$SUT_FILE' 'std::vector<int> scale_vec' 2 10" \
    "ASKELETON_HOME='$ASKELETON_HOME' '$ASKELETON_BIN' -p '$BUILD_PATH' --seed=123 --profile=boundary --out-dir '$OUT_DIR/08_boundary' '$SUT_FILE' && ASKELETON_HOME='$ASKELETON_HOME' '$ASKELETON_BIN' -p '$BUILD_PATH' --seed=123 --profile=safe --out-dir '$OUT_DIR/08_safe' '$SUT_FILE' && ASKELETON_HOME='$ASKELETON_HOME' '$ASKELETON_BIN' -p '$BUILD_PATH' --seed=123 --profile=stress --out-dir '$OUT_DIR/08_stress' '$SUT_FILE'" \
    "echo '[Boundary cfg]'; sed -n '1,120p' '$OUT_DIR/08_boundary/sut_showcase/sut_showcase.cfg'; echo; echo '[Stress cfg]'; sed -n '1,120p' '$OUT_DIR/08_stress/sut_showcase/sut_showcase.cfg'"

  run_guided_step \
    "09" \
    "Logs y Reportes" \
    "Salida machine-readable para CI/auditoría." \
    "show_near '$SUT_FILE' 'namespace showcase' 2 10" \
    "ASKELETON_HOME='$ASKELETON_HOME' '$ASKELETON_BIN' -p '$BUILD_PATH' --verbose --report-json --log-json='$OUT_DIR/09_log.json' --out-dir '$OUT_DIR/09_reports' '$SUT_FILE'" \
    "echo '[Report json]'; sed -n '1,220p' '$OUT_DIR/09_reports/askeleton_report.json' | rg -n 'coverage_mode|oracle_mode|summary|by_reason|generated|skipped' || true; echo; echo '[Log json]'; sed -n '1,220p' '$OUT_DIR/09_log.json'"

  run_guided_step \
    "10" \
    "Bootstrap CompDB" \
    "Demostración autocontenida cuando no hay compile_commands.json." \
    "echo 'int twice(int x){return x*2;}' > '$OUT_DIR/10_bootstrap_sut.cpp' && nl -ba '$OUT_DIR/10_bootstrap_sut.cpp'" \
    "ASKELETON_HOME='$ASKELETON_HOME' '$ASKELETON_BIN' --bootstrap-compdb -p '$OUT_DIR' --out-dir '$OUT_DIR/10_bootstrap' '$OUT_DIR/10_bootstrap_sut.cpp'" \
    "echo '[compdb creado]'; sed -n '1,120p' '$OUT_DIR/compile_commands.json'; echo; echo '[artefactos]'; find '$OUT_DIR/10_bootstrap' -maxdepth 2 -type f | sort"
}

run_quick_basic() {
  run_guided_step \
    "01" \
    "Baseline Basic" \
    "Generación por defecto sobre sut.cpp." \
    "nl -ba '$SUT_FILE' | sed -n '1,160p'" \
    "ASKELETON_HOME='$ASKELETON_HOME' '$ASKELETON_BIN' -p '$BUILD_PATH' --out-dir '$OUT_DIR/01_default' '$SUT_FILE'" \
    "find '$OUT_DIR/01_default' -maxdepth 2 -type f | sort | sed -n '1,30p'; echo; sed -n '1,140p' '$OUT_DIR/01_default/sut/sut_test.cpp'"

  run_guided_step \
    "02" \
    "Strict + Report" \
    "Comparación de skips en strict." \
    "nl -ba '$SUT_FILE' | sed -n '1,200p'" \
    "ASKELETON_HOME='$ASKELETON_HOME' '$ASKELETON_BIN' -p '$BUILD_PATH' --coverage-mode=strict --report='$OUT_DIR/02_strict_report.json' --out-dir '$OUT_DIR/02_strict' '$SUT_FILE'" \
    "sed -n '1,220p' '$OUT_DIR/02_strict_report.json' | rg -n 'by_reason|skipped|coverage_' || true"
}

print_final_guide() {
  print_sep
  cat <<'EOF'
Guía rápida final (ES)

Modos de la demo:
- `--quick`: recorrido corto guiado, centrado en lo más didáctico.
- `--full`: añade pasos extra (perfiles, logs/reportes, bootstrap, etc.).
- `--sut showcase`: usa `sut_showcase.cpp` (cobertura amplia y skips intencionados).
- `--sut basic`: usa `sut.cpp` (demo mínima).

Modos/opciones de ASkeleTon que verás en la demo:
- `--framework=gtest|boost|catch`: cambia el framework de pruebas generado.
- `--profile=random|boundary|safe|stress`: cambia el estilo de datos de entrada.
- `--coverage-mode=balanced|strict|aggressive`: política de generación/cobertura.
- `--oracle-mode=explicit|mirror|property`: estrategia para calcular/comparar expected.
- `--rule-data` / `--no-rule-data`: activa/desactiva valores guiados por reglas AST.
- `--seed=N`: hace la generación determinista/reproducible.
- `--report=...` o `--report-json`: genera resumen JSON de generados/skips.
- `--log-json=...`: guarda log de ejecución detallado.
- `--bootstrap-compdb`: crea `compile_commands.json` mínimo si falta.

Razones de skip (las más comunes en este showcase):
- `abstract_record`: tipo abstracto (no se puede instanciar para el fixture).
- `unsupported_indirection`: demasiada indireccion en punteros/referencias.
- `unsupported_type_shape`: forma de tipo no soportada por el materializador.
- `coverage_policy_mutable_parameter`: en `strict`, se omiten params mutables.
- `coverage_policy_instance_construction`: en `strict`, se omite instanciación no default.

Dónde mirar:
- Consola: resumen Found/Generated/Skipped.
- `*_report.json`: detalle machine-readable de razones de skip.
- `*_test.cpp` y `*.cfg`: pruebas generadas y datos de entrada/oráculo.
EOF
}

show_header
step_intro_only
if [[ "$SUT_KIND" == "showcase" ]]; then
  run_quick_showcase
  if [[ "$MODE" == "full" ]]; then
    run_full_extras_showcase
  fi
else
  run_quick_basic
fi

print_final_guide
print_sep
echo "Demo completada."
echo "Material generado en: $OUT_DIR"
