# Intelligent CPU Scheduling Analyzer
A modular C++ simulator to visualize, analyze, and recommend CPU scheduling algorithms.

## 🚀 Features
- **6 Algorithms:** FCFS, Round Robin, SPN, SRT, HRRN, and Multi-Level Feedback Queues.
- **Visual Gantt Charts:** Real-time generation of task execution flows.
- **Intelligent Recommendation Engine:** Uses **Min-Max Normalized Scoring** to pick the best algorithm based on efficiency and fairness.
- **Metric Suite:** Calculates Turnaround Time, Waiting Time, and Fairness (Standard Deviation).

## 🛠️ How to Run
1. Compile: `g++ src/simulator.cpp -o simulator`
2. Run: `./simulator`