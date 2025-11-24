# ASTERIX Radar Integration - Jupyter Notebooks

Interactive tutorials demonstrating ASTERIX radar integration capabilities.

## 📓 Notebooks

### 01_Quick_Start.ipynb
**Difficulty:** Beginner
**Duration:** 15-20 minutes

Learn the basics:
- Creating a mock radar
- Generating radar plots
- ASCII visualization
- ASTERIX encoding
- Statistics analysis

**Perfect for:** First-time users, quick introduction

---

### 02_Multi_Category_Encoding.ipynb (Coming Soon)
**Difficulty:** Intermediate
**Duration:** 30-40 minutes

Explore all 7 ASTERIX categories:
- CAT001, 019, 020, 021, 034, 048, 062
- Different surveillance types
- Precision comparison
- Complete airport scenario

---

### 03_Flight_Simulator_Integration.ipynb
**Difficulty:** Advanced
**Duration:** 45-60 minutes

Flight simulator integration:
- ArduPilot SITL setup and MAVLink telemetry
- JSBSim aircraft simulation
- Real-time vs batch processing
- Flight trajectory → ASTERIX CAT021
- Field mapping tables
- Comparison and use cases

---

### 04_Advanced_Visualization.ipynb (Coming Soon)
**Difficulty:** Intermediate
**Duration:** 30-40 minutes

Visualization features:
- Matplotlib PPI plots
- Track visualization
- SNR heatmaps
- Coverage analysis

---

### 05_Real_Time_Streaming.ipynb (Coming Soon)
**Difficulty:** Advanced
**Duration:** 40-50 minutes

Real-time capabilities:
- Live radar simulation
- UDP multicast streaming
- Multi-category workflows
- System integration

---

## 🚀 Getting Started

### Install Jupyter

```bash
pip install jupyter notebook
# Or use JupyterLab
pip install jupyterlab
```

### Launch Notebooks

```bash
cd examples/radar_integration/notebooks
jupyter notebook
```

Then open `01_Quick_Start.ipynb` in your browser.

---

## 📚 Requirements

**Required:**
- ASTERIX decoder package installed
- Python 3.10+
- numpy (included with ASTERIX)

**Optional:**
- matplotlib (for advanced visualization notebooks)
- pymavlink (for ArduPilot integration notebook)
- jsbsim (for JSBSim integration notebook)

---

## 🎯 Learning Path

1. **Beginner:** Start with `01_Quick_Start.ipynb`
2. **Intermediate:** Try `02_Multi_Category_Encoding.ipynb`
3. **Advanced:** Explore `03_Flight_Simulator_Integration.ipynb`
4. **Visualization:** Check out `04_Advanced_Visualization.ipynb`
5. **Real-Time:** Master `05_Real_Time_Streaming.ipynb`

---

## 💡 Tips

- **Run cells in order** - Later cells depend on earlier ones
- **Experiment!** - Modify parameters and see what happens
- **Check output** - Each cell shows results immediately
- **Save your work** - Notebooks auto-save, but export important results

---

## 🐛 Troubleshooting

**Import errors:**
```python
# Add ASTERIX to path
import sys
sys.path.insert(0, '/path/to/asterix')
```

**Visualization not showing:**
```python
# Enable inline plots
%matplotlib inline
```

**Module not found:**
```bash
# Install ASTERIX package
cd /path/to/asterix
python3 setup.py install --user
```

---

## 📖 Documentation

- **Main Guide:** `../../docs/INTEGRATION_RADAR_SIMULATION.md`
- **API Reference:** Docstrings in code
- **Examples:** `../` directory

---

**Happy learning!** 🎓
