#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ASKELETON_BIN="${ROOT_DIR}/askeleton"
export ASKELETON_HOME="${ASKELETON_HOME:-$ROOT_DIR}"

MODE="full"
SUT_KIND="showcase"
OUT_DIR="${ROOT_DIR}/demo_runs/$(date +%Y%m%d_%H%M%S)"

usage() {
  cat <<'USAGE'
Uso:
  ./scripts/demo_askeleton.sh [--quick|--full] [--sut showcase|basic] [--out DIR]

Opciones:
  --quick            Demo corta (pasos clave)
  --full             Demo completa (muchos modos/flags) [por defecto]
  --sut showcase     Usa examples/sut_showcase.cpp [por defecto]
  --sut basic        Usa examples/sut.cpp
  --out DIR          Carpeta base de salida
  --help             Muestra esta ayuda

Ejemplos:
  ./scripts/demo_askeleton.sh
  ./scripts/demo_askeleton.sh --quick
  ./scripts/demo_askeleton.sh --sut basic --out /tmp/demo_basic
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
      if [[ $# -lt 2 ]]; then
        echo "ERROR: falta valor para --sut"
        exit 1
      fi
      SUT_KIND="$2"
      shift 2
      ;;
    --out)
      if [[ $# -lt 2 ]]; then
        echo "ERROR: falta valor para --out"
        exit 1
      fi
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

if [[ ! -x "$ASKELETON_BIN" ]]; then
  echo "ERROR: no encuentro ejecutable en $ASKELETON_BIN"
  exit 1
fi

BUILD_PATH="${ROOT_DIR}/examples"
if [[ "$SUT_KIND" == "showcase" ]]; then
  SUT_FILE="${ROOT_DIR}/examples/sut_showcase.cpp"
elif [[ "$SUT_KIND" == "basic" ]]; then
  SUT_FILE="${ROOT_DIR}/examples/sut.cpp"
else
  echo "ERROR: --sut debe ser 'showcase' o 'basic'"
  exit 1
fi

if [[ ! -f "$SUT_FILE" ]]; then
  echo "ERROR: no encuentro el SUT en $SUT_FILE"
  exit 1
fi

mkdir -p "$OUT_DIR"

BOOTSTRAP_DIR="${OUT_DIR}/bootstrap_case"
INCLUDE_CASE_DIR="${OUT_DIR}/include_case"
mkdir -p "$BOOTSTRAP_DIR" "$INCLUDE_CASE_DIR/include"
cp "$SUT_FILE" "$BOOTSTRAP_DIR/sut_input.cpp"
cat > "$INCLUDE_CASE_DIR/include/impl_under_include.cpp" <<'CPP'
int mul2(int x) { return x * 2; }
CPP

pause_or_quit() {
  local answer
  read -r -p "Pulsa Enter para ejecutar (q para salir): " answer
  if [[ "$answer" == "q" || "$answer" == "Q" ]]; then
    echo "Demo interrumpida por el usuario."
    exit 0
  fi
}

run_step() {
  local id="$1"
  local title="$2"
  local explain="$3"
  local cmd="$4"
  local inspect_hint="$5"

  echo
  echo "================================================================"
  echo "[$id] $title"
  echo "$explain"
  echo "Comando:"
  echo "  $cmd"
  if [[ -n "$inspect_hint" ]]; then
    echo "Mirar después: $inspect_hint"
  fi
  echo "================================================================"

  pause_or_quit

  set +e
  bash -lc "$cmd"
  local status=$?
  set -e

  if [[ $status -ne 0 ]]; then
    echo "[WARN] El comando terminó con código $status"
  fi

  if [[ -n "$inspect_hint" ]]; then
    echo "Listo. Puedes enseñar ahora: $inspect_hint"
  fi
}

echo "Demo interactiva de ASkeleTon"
echo "Modo: $MODE"
echo "SUT: $SUT_FILE"
echo "Salida base: $OUT_DIR"
echo "ASKELETON_HOME: $ASKELETON_HOME"

run_common_intro() {
  run_step "00" "Version" \
    "Comprueba versión del binario que se va a usar en la demo." \
    "\"$ASKELETON_BIN\" --version" \
    "Consola"

  run_step "01" "Baseline" \
    "Generación base (gtest + defaults)." \
    "ASKELETON_HOME=\"$ASKELETON_HOME\" \"$ASKELETON_BIN\" -p \"$BUILD_PATH\" --out-dir \"$OUT_DIR/01_default\" \"$SUT_FILE\"" \
    "$OUT_DIR/01_default"
}

run_quick_steps() {
  run_step "02" "Skips en balanced (reporte JSON)" \
    "Aquí se ven skips estructurales del SUT showcase (si aplica)." \
    "ASKELETON_HOME=\"$ASKELETON_HOME\" \"$ASKELETON_BIN\" -p \"$BUILD_PATH\" --coverage-mode=balanced --report=\"$OUT_DIR/02_balanced_report.json\" --out-dir \"$OUT_DIR/02_balanced\" \"$SUT_FILE\"" \
    "$OUT_DIR/02_balanced_report.json"

  run_step "03" "Skips por política strict" \
    "En strict suelen aparecer skips por mutable params e instancia no default." \
    "ASKELETON_HOME=\"$ASKELETON_HOME\" \"$ASKELETON_BIN\" -p \"$BUILD_PATH\" --coverage-mode=strict --report=\"$OUT_DIR/03_strict_report.json\" --out-dir \"$OUT_DIR/03_strict\" \"$SUT_FILE\"" \
    "$OUT_DIR/03_strict_report.json"

  run_step "04" "Frameworks alternativos" \
    "Compara estructura generada entre boost y catch." \
    "ASKELETON_HOME=\"$ASKELETON_HOME\" \"$ASKELETON_BIN\" -p \"$BUILD_PATH\" --framework=boost --out-dir \"$OUT_DIR/04_boost\" \"$SUT_FILE\" && ASKELETON_HOME=\"$ASKELETON_HOME\" \"$ASKELETON_BIN\" -p \"$BUILD_PATH\" --framework=catch --out-dir \"$OUT_DIR/04_catch\" \"$SUT_FILE\"" \
    "$OUT_DIR/04_boost y $OUT_DIR/04_catch"

  run_step "05" "Perfiles y oráculo" \
    "Muestra que cambian los datos y la estrategia de expected values." \
    "ASKELETON_HOME=\"$ASKELETON_HOME\" \"$ASKELETON_BIN\" -p \"$BUILD_PATH\" --profile=boundary --oracle-mode=property --seed=123 --out-dir \"$OUT_DIR/05_profile_oracle\" \"$SUT_FILE\"" \
    "$OUT_DIR/05_profile_oracle"
}

run_full_steps() {
  run_step "02" "Framework boost" \
    "Mismo SUT, salida para Boost.Test." \
    "ASKELETON_HOME=\"$ASKELETON_HOME\" \"$ASKELETON_BIN\" -p \"$BUILD_PATH\" --framework=boost --out-dir \"$OUT_DIR/02_framework_boost\" \"$SUT_FILE\"" \
    "$OUT_DIR/02_framework_boost"

  run_step "03" "Framework catch" \
    "Mismo SUT, salida para Catch2." \
    "ASKELETON_HOME=\"$ASKELETON_HOME\" \"$ASKELETON_BIN\" -p \"$BUILD_PATH\" --framework=catch --out-dir \"$OUT_DIR/03_framework_catch\" \"$SUT_FILE\"" \
    "$OUT_DIR/03_framework_catch"

  run_step "04" "Profile boundary" \
    "Datos orientados a bordes." \
    "ASKELETON_HOME=\"$ASKELETON_HOME\" \"$ASKELETON_BIN\" -p \"$BUILD_PATH\" --profile=boundary --out-dir \"$OUT_DIR/04_profile_boundary\" \"$SUT_FILE\"" \
    "$OUT_DIR/04_profile_boundary"

  run_step "05" "Profile safe" \
    "Datos conservadores." \
    "ASKELETON_HOME=\"$ASKELETON_HOME\" \"$ASKELETON_BIN\" -p \"$BUILD_PATH\" --profile=safe --out-dir \"$OUT_DIR/05_profile_safe\" \"$SUT_FILE\"" \
    "$OUT_DIR/05_profile_safe"

  run_step "06" "Profile stress" \
    "Datos más agresivos." \
    "ASKELETON_HOME=\"$ASKELETON_HOME\" \"$ASKELETON_BIN\" -p \"$BUILD_PATH\" --profile=stress --out-dir \"$OUT_DIR/06_profile_stress\" \"$SUT_FILE\"" \
    "$OUT_DIR/06_profile_stress"

  run_step "07" "Coverage strict + reporte" \
    "Ideal para enseñar skips por política de cobertura." \
    "ASKELETON_HOME=\"$ASKELETON_HOME\" \"$ASKELETON_BIN\" -p \"$BUILD_PATH\" --coverage-mode=strict --report=\"$OUT_DIR/07_strict_report.json\" --out-dir \"$OUT_DIR/07_coverage_strict\" \"$SUT_FILE\"" \
    "$OUT_DIR/07_strict_report.json"

  run_step "08" "Coverage balanced + reporte" \
    "Comparación directa con strict." \
    "ASKELETON_HOME=\"$ASKELETON_HOME\" \"$ASKELETON_BIN\" -p \"$BUILD_PATH\" --coverage-mode=balanced --report=\"$OUT_DIR/08_balanced_report.json\" --out-dir \"$OUT_DIR/08_coverage_balanced\" \"$SUT_FILE\"" \
    "$OUT_DIR/08_balanced_report.json"

  run_step "09" "Coverage aggressive" \
    "Modo agresivo (forward-compatible)." \
    "ASKELETON_HOME=\"$ASKELETON_HOME\" \"$ASKELETON_BIN\" -p \"$BUILD_PATH\" --coverage-mode=aggressive --out-dir \"$OUT_DIR/09_coverage_aggressive\" \"$SUT_FILE\"" \
    "$OUT_DIR/09_coverage_aggressive"

  run_step "10" "Oracle mirror" \
    "Oráculo por replay espejo." \
    "ASKELETON_HOME=\"$ASKELETON_HOME\" \"$ASKELETON_BIN\" -p \"$BUILD_PATH\" --oracle-mode=mirror --out-dir \"$OUT_DIR/10_oracle_mirror\" \"$SUT_FILE\"" \
    "$OUT_DIR/10_oracle_mirror"

  run_step "11" "Oracle explicit" \
    "Oráculo con expected explícito + fallback." \
    "ASKELETON_HOME=\"$ASKELETON_HOME\" \"$ASKELETON_BIN\" -p \"$BUILD_PATH\" --oracle-mode=explicit --out-dir \"$OUT_DIR/11_oracle_explicit\" \"$SUT_FILE\"" \
    "$OUT_DIR/11_oracle_explicit"

  run_step "12" "Oracle property" \
    "Oráculo de repetibilidad." \
    "ASKELETON_HOME=\"$ASKELETON_HOME\" \"$ASKELETON_BIN\" -p \"$BUILD_PATH\" --oracle-mode=property --out-dir \"$OUT_DIR/12_oracle_property\" \"$SUT_FILE\"" \
    "$OUT_DIR/12_oracle_property"

  run_step "13" "Rule data ON/OFF" \
    "Compara generación con y sin reglas AST-guided." \
    "ASKELETON_HOME=\"$ASKELETON_HOME\" \"$ASKELETON_BIN\" -p \"$BUILD_PATH\" --rule-data --rule-max-cases=5 --out-dir \"$OUT_DIR/13_rule_data_on\" \"$SUT_FILE\" && ASKELETON_HOME=\"$ASKELETON_HOME\" \"$ASKELETON_BIN\" -p \"$BUILD_PATH\" --no-rule-data --out-dir \"$OUT_DIR/13_rule_data_off\" \"$SUT_FILE\"" \
    "$OUT_DIR/13_rule_data_on y $OUT_DIR/13_rule_data_off"

  run_step "14" "Seed reproducible (dos runs)" \
    "Dos ejecuciones con la misma seed para comparar estabilidad." \
    "ASKELETON_HOME=\"$ASKELETON_HOME\" \"$ASKELETON_BIN\" -p \"$BUILD_PATH\" --seed=123 --out-dir \"$OUT_DIR/14_seed_run1\" \"$SUT_FILE\" && ASKELETON_HOME=\"$ASKELETON_HOME\" \"$ASKELETON_BIN\" -p \"$BUILD_PATH\" --seed=123 --out-dir \"$OUT_DIR/14_seed_run2\" \"$SUT_FILE\"" \
    "$OUT_DIR/14_seed_run1 y $OUT_DIR/14_seed_run2"

  run_step "15" "Logs y report JSON" \
    "Salida de trazas y reporte machine-readable." \
    "ASKELETON_HOME=\"$ASKELETON_HOME\" \"$ASKELETON_BIN\" -p \"$BUILD_PATH\" --verbose --report-json --log-json=\"$OUT_DIR/15_run.log.json\" --out-dir \"$OUT_DIR/15_logs_reports\" \"$SUT_FILE\"" \
    "$OUT_DIR/15_logs_reports y $OUT_DIR/15_run.log.json"

  run_step "16" "Quiet + no-system-files-refresh" \
    "Ejemplo de ejecución silenciosa." \
    "ASKELETON_HOME=\"$ASKELETON_HOME\" \"$ASKELETON_BIN\" -p \"$BUILD_PATH\" --quiet --no-system-files-refresh --out-dir \"$OUT_DIR/16_quiet\" \"$SUT_FILE\"" \
    "$OUT_DIR/16_quiet"

  run_step "17" "Extra args + deep-level" \
    "Flags avanzados para clang tooling." \
    "ASKELETON_HOME=\"$ASKELETON_HOME\" \"$ASKELETON_BIN\" -p \"$BUILD_PATH\" --extra-arg-before=-DUSE_SHOWCASE=1 --extra-arg=-Wno-unused-parameter --deep-level=2 --out-dir \"$OUT_DIR/17_extra_deep\" \"$SUT_FILE\"" \
    "$OUT_DIR/17_extra_deep"

  run_step "18" "Bootstrap compdb" \
    "Genera compile_commands.json mínimo si no existe." \
    "ASKELETON_HOME=\"$ASKELETON_HOME\" \"$ASKELETON_BIN\" --bootstrap-compdb -p \"$BOOTSTRAP_DIR\" --out-dir \"$OUT_DIR/18_bootstrap\" \"$BOOTSTRAP_DIR/sut_input.cpp\"" \
    "$OUT_DIR/18_bootstrap"

  run_step "19" "Impl bajo include/ (sin flag)" \
    "Demuestra el filtro de seguridad por ubicación." \
    "ASKELETON_HOME=\"$ASKELETON_HOME\" \"$ASKELETON_BIN\" --bootstrap-compdb -p \"$INCLUDE_CASE_DIR\" --out-dir \"$OUT_DIR/19_include_blocked\" \"$INCLUDE_CASE_DIR/include/impl_under_include.cpp\"" \
    "$OUT_DIR/19_include_blocked"

  run_step "20" "Impl bajo include/ (con flag)" \
    "Mismo caso, forzado con --include-impl-under-include." \
    "ASKELETON_HOME=\"$ASKELETON_HOME\" \"$ASKELETON_BIN\" --bootstrap-compdb --include-impl-under-include -p \"$INCLUDE_CASE_DIR\" --out-dir \"$OUT_DIR/20_include_allowed\" \"$INCLUDE_CASE_DIR/include/impl_under_include.cpp\"" \
    "$OUT_DIR/20_include_allowed"
}

run_common_intro
if [[ "$MODE" == "quick" ]]; then
  run_quick_steps
else
  run_full_steps
fi

echo
echo "Demo completada."
echo "Todo el material quedó en: $OUT_DIR"
