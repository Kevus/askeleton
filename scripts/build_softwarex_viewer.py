#!/usr/bin/env python3
import csv
import json
import sys
from pathlib import Path


def load_csv(path: Path):
    with path.open("r", encoding="utf-8") as fh:
        return list(csv.DictReader(fh))


def main() -> int:
    if len(sys.argv) != 2:
        print("usage: build_publication_viewer.py <analysis_dir>", file=sys.stderr)
        return 1

    out_dir = Path(sys.argv[1]).resolve()
    data = {
        "baseline": load_csv(out_dir / "baseline_summary.csv"),
        "coverage": load_csv(out_dir / "coverage_ablation.csv"),
        "sensitivity": load_csv(out_dir / "full_factorial_sensitivity.csv"),
        "skips": load_csv(out_dir / "skip_reason_summary.csv"),
        "raw": load_csv(out_dir / "raw_runs.csv"),
    }

    html = f"""<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>publication Evaluation Viewer</title>
  <style>
    :root {{
      --bg: #f4efe6;
      --panel: #fffaf1;
      --ink: #1c1917;
      --muted: #6b6258;
      --line: #d8cdbd;
      --accent: #0f766e;
      --accent-2: #b45309;
      --good: #166534;
      --warn: #92400e;
      --bad: #991b1b;
    }}
    * {{ box-sizing: border-box; }}
    body {{
      margin: 0;
      font-family: Georgia, "Iowan Old Style", "Palatino Linotype", serif;
      color: var(--ink);
      background:
        radial-gradient(circle at top left, #efe2c6 0, transparent 28%),
        radial-gradient(circle at top right, #d7ebe7 0, transparent 24%),
        linear-gradient(180deg, #f7f2ea 0%, var(--bg) 100%);
    }}
    .wrap {{
      width: min(1400px, calc(100vw - 32px));
      margin: 24px auto 48px;
    }}
    .hero {{
      padding: 28px;
      border: 1px solid var(--line);
      background: linear-gradient(135deg, rgba(15,118,110,.1), rgba(180,83,9,.08));
      box-shadow: 0 12px 30px rgba(28,25,23,.08);
      border-radius: 20px;
    }}
    h1, h2 {{
      margin: 0 0 10px;
      font-weight: 700;
      letter-spacing: -.02em;
    }}
    .hero p, .note {{
      color: var(--muted);
      margin: 8px 0 0;
      line-height: 1.45;
    }}
    .grid {{
      display: grid;
      grid-template-columns: repeat(12, 1fr);
      gap: 16px;
      margin-top: 18px;
    }}
    .card {{
      grid-column: span 12;
      background: var(--panel);
      border: 1px solid var(--line);
      border-radius: 18px;
      padding: 18px;
      box-shadow: 0 8px 24px rgba(28,25,23,.05);
    }}
    .stats {{
      display: grid;
      grid-template-columns: repeat(4, 1fr);
      gap: 12px;
      margin-top: 16px;
    }}
    .stat {{
      padding: 14px;
      border-radius: 14px;
      background: rgba(255,255,255,.7);
      border: 1px solid var(--line);
    }}
    .stat .k {{
      font-size: 12px;
      text-transform: uppercase;
      letter-spacing: .08em;
      color: var(--muted);
    }}
    .stat .v {{
      font-size: 28px;
      margin-top: 6px;
      font-weight: 700;
    }}
    .controls {{
      display: flex;
      flex-wrap: wrap;
      gap: 10px;
      margin-top: 12px;
      align-items: end;
    }}
    label {{
      display: grid;
      gap: 6px;
      font-size: 13px;
      color: var(--muted);
    }}
    select, input {{
      min-width: 180px;
      padding: 10px 12px;
      border-radius: 10px;
      border: 1px solid var(--line);
      background: #fffdf9;
      color: var(--ink);
      font: inherit;
    }}
    button {{
      padding: 10px 14px;
      border: 1px solid var(--line);
      border-radius: 10px;
      background: var(--ink);
      color: white;
      cursor: pointer;
      font: inherit;
    }}
    table {{
      width: 100%;
      border-collapse: collapse;
      margin-top: 12px;
      font-size: 14px;
    }}
    th, td {{
      padding: 10px 8px;
      border-bottom: 1px solid var(--line);
      text-align: left;
      vertical-align: top;
    }}
    th {{
      color: var(--muted);
      font-size: 12px;
      text-transform: uppercase;
      letter-spacing: .06em;
      position: sticky;
      top: 0;
      background: var(--panel);
    }}
    .table-wrap {{
      overflow: auto;
      max-height: 520px;
      border-top: 1px solid var(--line);
      margin-top: 12px;
    }}
    .pill {{
      display: inline-block;
      padding: 3px 8px;
      border-radius: 999px;
      font-size: 12px;
      border: 1px solid var(--line);
      background: rgba(15,118,110,.08);
      color: var(--accent);
      white-space: nowrap;
    }}
    .mono {{ font-family: ui-monospace, SFMono-Regular, Consolas, monospace; font-size: 12px; }}
    .good {{ color: var(--good); }}
    .warn {{ color: var(--warn); }}
    .bad {{ color: var(--bad); }}
    .split {{
      display: grid;
      grid-template-columns: 1.2fr .8fr;
      gap: 16px;
    }}
    @media (max-width: 980px) {{
      .stats, .split {{ grid-template-columns: 1fr; }}
    }}
  </style>
</head>
<body>
  <div class="wrap">
    <section class="hero">
      <h1>publication Evaluation Viewer</h1>
      <p>Interactive view over the current ASkeleTon evaluation package. All data are embedded in this file.</p>
      <div class="stats" id="stats"></div>
    </section>

    <section class="grid">
      <div class="card">
        <h2>Baseline</h2>
        <p class="note">Default window: <span class="pill">gtest</span> <span class="pill">seed 123</span> <span class="pill">profile random</span> <span class="pill">coverage balanced</span> <span class="pill">oracle explicit</span> <span class="pill">rule-data on</span></p>
        <div class="table-wrap" id="baseline"></div>
      </div>

      <div class="card">
        <h2>Coverage Ablation</h2>
        <div class="table-wrap" id="coverage"></div>
      </div>

      <div class="card split">
        <div>
          <h2>Full-Factorial Sensitivity</h2>
          <div class="controls">
            <label>Subject
              <select id="sens-subject"></select>
            </label>
          </div>
          <div class="table-wrap" id="sensitivity"></div>
        </div>
        <div>
          <h2>Top Skip Reasons</h2>
          <div class="table-wrap" id="skips"></div>
        </div>
      </div>

      <div class="card">
        <h2>Raw Runs</h2>
        <div class="controls">
          <label>Subject
            <select id="raw-subject"></select>
          </label>
          <label>Experiment
            <select id="raw-group"></select>
          </label>
          <label>Coverage
            <select id="raw-coverage"></select>
          </label>
          <label>Search
            <input id="raw-search" placeholder="run id, label, notes">
          </label>
          <button id="raw-reset" type="button">Reset</button>
        </div>
        <div class="table-wrap" id="raw"></div>
      </div>
    </section>
  </div>

  <script>
    const DATA = {json.dumps(data, ensure_ascii=False)};

    const pctClass = value => {{
      const n = Number(String(value).replace('%', ''));
      if (Number.isNaN(n)) return '';
      if (n >= 85) return 'good';
      if (n >= 50) return 'warn';
      return 'bad';
    }};

    function renderTable(targetId, rows, columns, opts = {{}}) {{
      const target = document.getElementById(targetId);
      let html = '<table><thead><tr>';
      for (const col of columns) html += `<th>${{col.label}}</th>`;
      html += '</tr></thead><tbody>';
      for (const row of rows) {{
        html += '<tr>';
        for (const col of columns) {{
          const raw = row[col.key] ?? '';
          const val = col.format ? col.format(raw, row) : raw;
          const cls = col.className ? col.className(raw, row) : '';
          html += `<td class="${{cls}}">${{val}}</td>`;
        }}
        html += '</tr>';
      }}
      if (!rows.length) {{
        html += `<tr><td colspan="${{columns.length}}" class="note">No rows match the current filters.</td></tr>`;
      }}
      html += '</tbody></table>';
      target.innerHTML = html;
    }}

    function uniqueValues(rows, key) {{
      return ['all', ...new Set(rows.map(r => r[key]).filter(Boolean))];
    }}

    function fillSelect(id, values) {{
      const el = document.getElementById(id);
      el.innerHTML = values.map(v => `<option value="${{v}}">${{v}}</option>`).join('');
    }}

    function renderStats() {{
      const raw = DATA.raw;
      const avg = raw.reduce((acc, r) => acc + Number(r.generation_rate), 0) / raw.length;
      const stats = [
        ['Runs', raw.length],
        ['Subjects', new Set(raw.map(r => r.subject)).size],
        ['Avg generation', `${{(avg * 100).toFixed(2)}}%`],
        ['Exit code 0', raw.filter(r => Number(r.exit_code) === 0).length],
      ];
      document.getElementById('stats').innerHTML = stats.map(([k,v]) => `
        <div class="stat"><div class="k">${{k}}</div><div class="v">${{v}}</div></div>
      `).join('');
    }}

    function renderBaseline() {{
      renderTable('baseline', DATA.baseline, [
        {{ key: 'label', label: 'Subject' }},
        {{ key: 'found', label: 'Found' }},
        {{ key: 'generated', label: 'Generated' }},
        {{ key: 'skipped', label: 'Skipped' }},
        {{ key: 'generation_rate', label: 'Generation rate', className: v => pctClass(v) }},
      ]);
    }}

    function renderCoverage() {{
      renderTable('coverage', DATA.coverage, [
        {{ key: 'label', label: 'Subject' }},
        {{ key: 'strict_rate', label: 'Strict', className: v => pctClass(v) }},
        {{ key: 'balanced_rate', label: 'Balanced', className: v => pctClass(v) }},
        {{ key: 'aggressive_rate', label: 'Aggressive', className: v => pctClass(v) }},
        {{ key: 'strict_generated', label: 'Strict gen' }},
        {{ key: 'balanced_generated', label: 'Balanced gen' }},
        {{ key: 'aggressive_generated', label: 'Aggressive gen' }},
      ]);
    }}

    function renderSensitivity() {{
      const subject = document.getElementById('sens-subject').value;
      const rows = DATA.sensitivity.filter(r => subject === 'all' || r.subject === subject);
      renderTable('sensitivity', rows, [
        {{ key: 'subject', label: 'Subject' }},
        {{ key: 'dimension', label: 'Dimension' }},
        {{ key: 'level', label: 'Level' }},
        {{ key: 'avg_generation_rate', label: 'Avg rate', className: v => pctClass(v) }},
        {{ key: 'min_generation_rate', label: 'Min rate', className: v => pctClass(v) }},
        {{ key: 'max_generation_rate', label: 'Max rate', className: v => pctClass(v) }},
        {{ key: 'runs', label: 'Runs' }},
      ]);
    }}

    function renderSkips() {{
      renderTable('skips', DATA.skips, [
        {{ key: 'reason', label: 'Reason', className: () => 'mono' }},
        {{ key: 'count', label: 'Count' }},
      ]);
    }}

    function renderRaw() {{
      const subject = document.getElementById('raw-subject').value;
      const group = document.getElementById('raw-group').value;
      const coverage = document.getElementById('raw-coverage').value;
      const search = document.getElementById('raw-search').value.trim().toLowerCase();
      const rows = DATA.raw.filter(r => {{
        if (subject !== 'all' && r.subject !== subject) return false;
        if (group !== 'all' && r.experiment_group !== group) return false;
        if (coverage !== 'all' && r.coverage_mode !== coverage) return false;
        if (search) {{
          const hay = [r.run_id, r.label, r.notes].join(' ').toLowerCase();
          if (!hay.includes(search)) return false;
        }}
        return true;
      }});
      renderTable('raw', rows, [
        {{ key: 'run_id', label: 'Run', className: () => 'mono' }},
        {{ key: 'label', label: 'Subject' }},
        {{ key: 'experiment_group', label: 'Experiment' }},
        {{ key: 'coverage_mode', label: 'Coverage' }},
        {{ key: 'profile', label: 'Profile' }},
        {{ key: 'oracle_mode', label: 'Oracle' }},
        {{ key: 'rule_data', label: 'Rule data' }},
        {{ key: 'found', label: 'Found' }},
        {{ key: 'generated', label: 'Generated' }},
        {{ key: 'skipped', label: 'Skipped' }},
        {{ key: 'generation_rate', label: 'Rate', format: v => `${{(Number(v) * 100).toFixed(2)}}%`, className: v => pctClass(Number(v) * 100) }},
        {{ key: 'report_path', label: 'Report', className: () => 'mono' }},
      ]);
    }}

    function init() {{
      renderStats();
      renderBaseline();
      renderCoverage();
      renderSkips();

      fillSelect('sens-subject', uniqueValues(DATA.sensitivity, 'subject'));
      fillSelect('raw-subject', uniqueValues(DATA.raw, 'subject'));
      fillSelect('raw-group', uniqueValues(DATA.raw, 'experiment_group'));
      fillSelect('raw-coverage', uniqueValues(DATA.raw, 'coverage_mode'));

      document.getElementById('sens-subject').addEventListener('change', renderSensitivity);
      for (const id of ['raw-subject', 'raw-group', 'raw-coverage']) {{
        document.getElementById(id).addEventListener('change', renderRaw);
      }}
      document.getElementById('raw-search').addEventListener('input', renderRaw);
      document.getElementById('raw-reset').addEventListener('click', () => {{
        document.getElementById('raw-subject').value = 'all';
        document.getElementById('raw-group').value = 'all';
        document.getElementById('raw-coverage').value = 'all';
        document.getElementById('raw-search').value = '';
        renderRaw();
      }});

      renderSensitivity();
      renderRaw();
    }}

    init();
  </script>
</body>
</html>
"""

    (out_dir / "viewer.html").write_text(html, encoding="utf-8")
    print(out_dir / "viewer.html")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
