# ─────────────────────────────────────────────────────────────
#  server.py
#  Flask backend that bridges the Terminator UI to the real
#  C++ Alan compiler. Accepts POST requests with Alan source
#  code, runs ./compiler, and returns stdout/stderr.
#
#  Usage:
#    pip install flask flask-cors
#    python server.py
#    Then open terminator_ui.html in your browser
#
#  Generated with Claude assistance — May 11, 2025
# ─────────────────────────────────────────────────────────────

from flask import Flask, request, jsonify, send_from_directory
from flask_cors import CORS
import subprocess
import tempfile
import os

BASE_DIR = os.path.dirname(os.path.abspath(__file__))


app = Flask(__name__)
CORS(app)  # Allow browser requests from file:// or localhost

# Serve the HTML file directly
@app.route('/')
def index():
    return send_from_directory(BASE_DIR, 'terminator_ui_live.html')

@app.route('/compile', methods=['POST'])
def compile_source():
    data = request.get_json()
    if not data or 'source' not in data:
        return jsonify({'error': 'No source provided'}), 400

    source = data['source']

    # Write source to a temp file
    with tempfile.NamedTemporaryFile(
        mode='w', suffix='.txt', delete=False, dir='/tmp'
    ) as f:
        f.write(source)
        tmpfile = f.name

    try:
        # Run the actual C++ compiler
        result = subprocess.run(
            ['../compiler', tmpfile],
            capture_output=True,
            text=True,
            timeout=15,
            cwd=os.path.dirname(os.path.abspath(__file__))
        )
        return jsonify({
            'stdout': result.stdout,
            'stderr': result.stderr,
            'returncode': result.returncode
        })
    except subprocess.TimeoutExpired:
        return jsonify({'error': 'Compiler timed out', 'returncode': 1}), 200
    except FileNotFoundError:
        return jsonify({'error': './compiler not found — run make first', 'returncode': 1}), 200
    finally:
        os.unlink(tmpfile)


if __name__ == '__main__':
    print('\n  C++ TERMINATOR COMPILER — BACKEND SERVER')
    print('  ─────────────────────────────────────────')
    print('  Running on http://localhost:5000')
    print('  Open http://localhost:5000 in your browser')
    print('  Press Ctrl+C to stop\n')
    app.run(port=5000, debug=False)