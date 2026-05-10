#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include <queue>
#include <climits>
#include <cfloat>
#include <cmath>

using namespace std;

struct Process {
    int id;
    int arrivalTime;
    int burstTime;
    int remainingTime;
    int completionTime = 0;
    int turnAroundTime = 0;
    int waitingTime = 0;
    int priorityLevel = 0; // For Feedback Queues
};

struct AlgoResult {
    string name;
    float avgWT;
    float avgTAT;
    float fairness; 
    float score = 0;
};

vector<pair<int, int>> ganttData;

void printGanttChart() {
    if(ganttData.empty()) return;
    cout << "\n--- Visual Gantt Chart ---\n ";
    for (auto p : ganttData) cout << "-------";
    cout << "\n|";
    for (auto p : ganttData) {
        string label = "P" + to_string(p.first);
        cout << "  " << left << setw(3) << label << " |";
    }
    cout << "\n ";
    for (auto p : ganttData) cout << "-------";
    cout << "\n" << endl;
}

vector<Process> resetProcesses(const vector<Process>& original) {
    ganttData.clear();
    vector<Process> reset = original;
    for (auto& p : reset) p.remainingTime = p.burstTime;
    return reset;
}

float calculateFairness(const vector<Process>& proc) {
    float sum = 0, mean, variance = 0;
    for(auto p : proc) sum += p.waitingTime;
    mean = sum / proc.size();
    for(auto p : proc) variance += pow(p.waitingTime - mean, 2);
    return sqrt(variance / proc.size()); 
}

// --- 1. FCFS ---
AlgoResult runFCFS(vector<Process> proc) {
    int currentTime = 0;
    sort(proc.begin(), proc.end(), [](Process a, Process b) { return a.arrivalTime < b.arrivalTime; });
    for (auto& p : proc) {
        if (currentTime < p.arrivalTime) currentTime = p.arrivalTime;
        ganttData.push_back({p.id, p.burstTime});
        p.waitingTime = currentTime - p.arrivalTime;
        currentTime += p.burstTime;
        p.turnAroundTime = p.waitingTime + p.burstTime;
    }
    float totalWT = 0, totalTAT = 0;
    for(auto p : proc) { totalWT += p.waitingTime; totalTAT += p.turnAroundTime; }
    cout << ">> FCFS Execution Flow:"; printGanttChart();
    return {"FCFS", totalWT / proc.size(), totalTAT / proc.size(), calculateFairness(proc)};
}

// --- 2. RR ---
AlgoResult runRR(vector<Process> proc, int quantum) {
    int n = proc.size(), currentTime = 0, completed = 0;
    queue<int> q;
    vector<bool> inQueue(n, false);
    sort(proc.begin(), proc.end(), [](Process a, Process b) { return a.arrivalTime < b.arrivalTime; });
    q.push(0); inQueue[0] = true;
    while (completed < n) {
        if (q.empty()) {
            currentTime++;
            for(int i=0; i<n; i++) if(proc[i].arrivalTime <= currentTime && proc[i].remainingTime > 0) { q.push(i); inQueue[i]=true; break; }
            continue;
        }
        int idx = q.front(); q.pop();
        int exec = min(proc[idx].remainingTime, quantum);
        ganttData.push_back({proc[idx].id, exec});
        proc[idx].remainingTime -= exec;
        currentTime += exec;
        for(int i=0; i<n; i++) if(proc[i].arrivalTime <= currentTime && !inQueue[i] && proc[i].remainingTime > 0) { q.push(i); inQueue[i]=true; }
        if (proc[idx].remainingTime > 0) q.push(idx);
        else {
            completed++;
            proc[idx].turnAroundTime = currentTime - proc[idx].arrivalTime;
            proc[idx].waitingTime = proc[idx].turnAroundTime - proc[idx].burstTime;
        }
    }
    float totalWT = 0, totalTAT = 0;
    for(auto p : proc) { totalWT += p.waitingTime; totalTAT += p.turnAroundTime; }
    cout << ">> RR Execution Flow:"; printGanttChart();
    return {"Round Robin", totalWT / n, totalTAT / n, calculateFairness(proc)};
}

// --- 3. SPN ---
AlgoResult runSPN(vector<Process> proc) {
    int n = proc.size(), completed = 0, currentTime = 0;
    vector<bool> isCompleted(n, false);
    while (completed < n) {
        int idx = -1; int minBurst = INT_MAX;
        for (int i = 0; i < n; i++) if (proc[i].arrivalTime <= currentTime && !isCompleted[i]) if (proc[i].burstTime < minBurst) { minBurst = proc[i].burstTime; idx = i; }
        if (idx != -1) {
            ganttData.push_back({proc[idx].id, proc[idx].burstTime});
            proc[idx].waitingTime = currentTime - proc[idx].arrivalTime;
            currentTime += proc[idx].burstTime;
            proc[idx].turnAroundTime = proc[idx].waitingTime + proc[idx].burstTime;
            isCompleted[idx] = true; completed++;
        } else currentTime++;
    }
    float totalWT = 0, totalTAT = 0;
    for(auto p : proc) { totalWT += p.waitingTime; totalTAT += p.turnAroundTime; }
    cout << ">> SPN Execution Flow:"; printGanttChart();
    return {"SPN", totalWT / n, totalTAT / n, calculateFairness(proc)};
}

// --- 4. SRT ---
AlgoResult runSRT(vector<Process> proc) {
    int n = proc.size(), completed = 0, currentTime = 0, minRem = INT_MAX, shortest = 0;
    bool check = false;
    while (completed < n) {
        for (int i = 0; i < n; i++) if (proc[i].arrivalTime <= currentTime && proc[i].remainingTime < minRem && proc[i].remainingTime > 0) { minRem = proc[i].remainingTime; shortest = i; check = true; }
        if (!check) { currentTime++; continue; }
        if(ganttData.empty() || ganttData.back().first != proc[shortest].id) ganttData.push_back({proc[shortest].id, 1});
        proc[shortest].remainingTime--;
        minRem = (proc[shortest].remainingTime == 0) ? INT_MAX : proc[shortest].remainingTime;
        if (proc[shortest].remainingTime == 0) {
            completed++; check = false;
            proc[shortest].turnAroundTime = (currentTime + 1) - proc[shortest].arrivalTime;
            proc[shortest].waitingTime = proc[shortest].turnAroundTime - proc[shortest].burstTime;
        }
        currentTime++;
    }
    float totalWT = 0, totalTAT = 0;
    for(auto p : proc) { totalWT += p.waitingTime; totalTAT += p.turnAroundTime; }
    cout << ">> SRT Execution Flow:"; printGanttChart();
    return {"SRT", totalWT / n, totalTAT / n, calculateFairness(proc)};
}

// --- 5. HRRN ---
AlgoResult runHRRN(vector<Process> proc) {
    int n = proc.size(), completed = 0, currentTime = 0;
    vector<bool> isCompleted(n, false);
    while (completed < n) {
        int idx = -1; float maxRR = -1.0;
        for (int i = 0; i < n; i++) if (proc[i].arrivalTime <= currentTime && !isCompleted[i]) {
            float rr = (float)((currentTime - proc[i].arrivalTime) + proc[i].burstTime) / proc[i].burstTime;
            if (rr > maxRR) { maxRR = rr; idx = i; }
        }
        if (idx != -1) {
            ganttData.push_back({proc[idx].id, proc[idx].burstTime});
            proc[idx].waitingTime = currentTime - proc[idx].arrivalTime;
            currentTime += proc[idx].burstTime;
            proc[idx].turnAroundTime = proc[idx].waitingTime + proc[idx].burstTime;
            isCompleted[idx] = true; completed++;
        } else currentTime++;
    }
    float totalWT = 0, totalTAT = 0;
    for(auto p : proc) { totalWT += p.waitingTime; totalTAT += p.turnAroundTime; }
    cout << ">> HRRN Execution Flow:"; printGanttChart();
    return {"HRRN", totalWT / n, totalTAT / n, calculateFairness(proc)};
}

// --- 6. Feedback Queue (Simple 2-Level MLFQ) ---
AlgoResult runFeedback(vector<Process> proc, int q1) {
    int n = proc.size(), currentTime = 0, completed = 0;
    queue<int> level1, level2; // Two priority levels
    sort(proc.begin(), proc.end(), [](Process a, Process b) { return a.arrivalTime < b.arrivalTime; });
    
    int nextArr = 0;
    while (completed < n) {
        while(nextArr < n && proc[nextArr].arrivalTime <= currentTime) { level1.push(nextArr++); }

        if (!level1.empty()) {
            int idx = level1.front(); level1.pop();
            int exec = min(proc[idx].remainingTime, q1);
            ganttData.push_back({proc[idx].id, exec});
            proc[idx].remainingTime -= exec;
            currentTime += exec;
            while(nextArr < n && proc[nextArr].arrivalTime <= currentTime) { level1.push(nextArr++); }
            if (proc[idx].remainingTime > 0) level2.push(idx); // Demote to level 2
            else { completed++; proc[idx].turnAroundTime = currentTime - proc[idx].arrivalTime; proc[idx].waitingTime = proc[idx].turnAroundTime - proc[idx].burstTime; }
        } else if (!level2.empty()) {
            int idx = level2.front(); level2.pop();
            ganttData.push_back({proc[idx].id, proc[idx].remainingTime});
            currentTime += proc[idx].remainingTime;
            proc[idx].remainingTime = 0;
            while(nextArr < n && proc[nextArr].arrivalTime <= currentTime) { level1.push(nextArr++); }
            completed++; proc[idx].turnAroundTime = currentTime - proc[idx].arrivalTime; proc[idx].waitingTime = proc[idx].turnAroundTime - proc[idx].burstTime;
        } else currentTime++;
    }
    float totalWT = 0, totalTAT = 0;
    for(auto p : proc) { totalWT += p.waitingTime; totalTAT += p.turnAroundTime; }
    cout << ">> Feedback Queue Execution Flow:"; printGanttChart();
    return {"Feedback", totalWT / n, totalTAT / n, calculateFairness(proc)};
}

// --- NORMALIZED RECOMMENDATION ENGINE ---
void getRecommendation(vector<AlgoResult>& results) {
    cout << string(65, '=') << "\n   INTELLIGENT RECOMMENDATION ENGINE (NORMALIZED SCORING)\n" << string(65, '=') << endl;
    
    float minWT = FLT_MAX, maxWT = -1, minTAT = FLT_MAX, maxTAT = -1, minF = FLT_MAX, maxF = -1;
    
    // Find Min and Max for normalization
    for(auto r : results) {
        if(r.avgWT < minWT) minWT = r.avgWT; if(r.avgWT > maxWT) maxWT = r.avgWT;
        if(r.avgTAT < minTAT) minTAT = r.avgTAT; if(r.avgTAT > maxTAT) maxTAT = r.avgTAT;
        if(r.fairness < minF) minF = r.fairness; if(r.fairness > maxF) maxF = r.fairness;
    }

    float bestScore = FLT_MAX; string winner;
    for (auto& r : results) {
        // Normalizing values (Safety check to avoid division by zero)
        float nWT = (maxWT == minWT) ? 0 : (r.avgWT - minWT) / (maxWT - minWT);
        float nTAT = (maxTAT == minTAT) ? 0 : (r.avgTAT - minTAT) / (maxTAT - minTAT);
        float nF = (maxF == minF) ? 0 : (r.fairness - minF) / (maxF - minF);

        // Efficiency (80%) vs Fairness (20%)
        r.score = (nWT * 0.5) + (nTAT * 0.3) + (nF * 0.2);
        
        cout << left << setw(12) << r.name << " | Norm Score: " << fixed << setprecision(2) << r.score << " (Lower is Better)" << endl;
        
        if (r.score < bestScore) { 
            bestScore = r.score; 
            winner = r.name; 
        }
    }
    cout << "\n>>> RECOMMENDATION: " << winner << " is the most balanced algorithm for this workload. <<<\n" << endl;
}
int main() {
    int n, q;
    cout << "Enter number of processes: "; cin >> n;
    vector<Process> proc;
    for(int i=0; i<n; i++) {
        int a, b; cout << "P" << i+1 << " (Arrival Burst): "; cin >> a >> b;
        proc.push_back({i+1, a, b, b});
    }
    cout << "Enter Base Time Quantum: "; cin >> q;

    vector<AlgoResult> res;
    res.push_back(runFCFS(resetProcesses(proc)));
    res.push_back(runRR(resetProcesses(proc), q));
    res.push_back(runSPN(resetProcesses(proc)));
    res.push_back(runSRT(resetProcesses(proc)));
    res.push_back(runHRRN(resetProcesses(proc)));
    res.push_back(runFeedback(resetProcesses(proc), q));

    cout << "\n" << left << setw(12) << "ALGORITHM" << setw(10) << "AVG WT" << setw(10) << "AVG TAT" << setw(10) << "FAIRNESS" << endl;
    for (const auto& r : res) cout << left << setw(12) << r.name << setw(10) << r.avgWT << setw(10) << r.avgTAT << setw(10) << r.fairness << endl;

    getRecommendation(res);
    return 0;
}