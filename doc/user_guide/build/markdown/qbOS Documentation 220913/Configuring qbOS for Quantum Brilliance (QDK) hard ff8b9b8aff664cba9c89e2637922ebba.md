# Configuring qbOS for Quantum Brilliance (QDK) hardware

# 1.0 Background

This document is intended for a **site administrator** who has a  Quantum Brilliance hardware QPU (eg. QDK) installed at their site.  Typically each site will have **DNS information** that needs to be input into qbOS.  

Other configurable settings include:

- the **polling interval** (in seconds) when retrieving measurement shot outcomes
- the **polling maximum retry limit**
- the **over request factor for shots**
- the **sample-with-replacement** function (disabling it, or activating it above a given threshold)

## 1.1 `qbos_core.qpu_config`

Default value [string]: `"/mnt/qb/share/qpu_config.json"` 

Sets the URL, shot request and polling parameters for QB hardware.  The **value** for this attribute is th**e path and filename for a JSON file** that conforms to the format described in **Section 3.0**

# 2.0 Safe limiting from qbOS when requesting shots from QB hardware

A **hard limit of 512 shots** is now enforced for any request sent to QB QDK.  This provides a **safe operating upper limit**.  If a user requests `sn` shots from QB hardware, where `sn` > 512 shots, then the request is automatically set/limited to 512 shots.  When used in combination with `recursive_request` (see below), then a **sequence of requests (each no larger than 512 shots) is issued** until a total of `sn` successful outcomes has been accumulated.

# 3.0 JSON configuration file for named hardware accelerators

These keys are in the JSON configuration file:`/mnt/qb/share/qpu_config.json`.  qbOS provides this JSON file pre-installed. 

Contents of the file `/mnt/qb/share/qpu_config.json`:

```json
{   "accs": [
    {"acc": "dqc_gen1", "url": "https://10.10.10.120:8443", "poll_secs": 6, "poll_retrys": 100, "over_request": 48, "recursive_request": true, "resample": false, "resample_above_percentage": 100}, 
    {"acc": "qdk_gen1", "url": "https://10.10.10.121:8443", "poll_secs": 6, "poll_retrys": 100, "over_request": 48, "recursive_request": true, "resample": false, "resample_above_percentage": 100}, 
    {"acc": "loopback", "url": "http://127.0.0.1:8000", "poll_secs": 1, "poll_retrys": 100, "over_request": 48, "recursive_request": true, "resample": false, "resample_above_percentage": 100}
    ]
}
```

- Keys for **general setup**
    - `"acc"` - name (string) for a hardware QPU

- Keys related to the **number of shots and requests**
    
    <aside>
    📎 In the table below, these variables have the following meanings:
    
    `sn` : the number of shots that a user has requested.  In qbOS, this is also referred to as the target number of successful measurement outcomes.
    
    `part_sn` : the accumulated number of successful measurement outcomes so far
    
    </aside>
    
    | Key name | Type | Description | Default |
    | --- | --- | --- | --- |
    | over_request | Integer | A factor to multiply the number of shots requested from the QB hardware by.  This addresses the fact that at least 30% of any requested number of shots on QB hardware will have indeterminate outcomes. | 8 |
    | recursive_request | Boolean | True: causes recursive requests of over_request * (sn - part_sn) shots until we have accumulated sn successful measurement outcomes.  Note: if over_request * (sn- part_sn)  >  512, then the request is set to 512 shots | True |
    | resample | Boolean | True: causes sample-with-replacement to be used to resample from the sampling distribution so as to return exactly sn outcomes to the user | False |
    | resample_above_percentage | Integer | If the percentage of successful measurements from a request exceeds resample_above_percentage, then the next request will use resample = True.  This feature allows the target sn shots to be reached without a large number of requests that only ask for a small number of shots to be run. | 50 |

- Keys related to **polling** for shot results
    - `"poll_secs"` - the **time (in seconds) between polling attempts** (when getting shot measurement results).
    - `"poll_retrys"` - the **maximum number of attempts** to poll for measurement results before the polling is declared to have failed.

- Keys related to **networking**
    - `"url"` - the **URL** and **port** that the hardware QPU listens on.

**Example**:

Note: running this example requires the `loopback` to be activated.  

- To set up the `loopback`:
    
    Open a Terminal in JupyterLab and run the command:
    
    `python3 /mnt/qb/bin/qbqe_if_model.py`
    
    ![Untitled](Untitled%201.png)
    
    **Note: returned data when using the `loopback` are synthetic** and do not correspond to the input circuit nor to the results on real QB hardware.
    

Here is an example that invokes the `loopback` dummy tester.  This `loopback` always returns 4 shots.  Here we request 16 shots.  qbOS will make recursive requests until 16 shots are accumulated.  All resampling is disabled in this example.

- First activate the `loopback` tester:

```bash
python3 /mnt/qb/bin/qbqe_if_model.py &

```

- Then run this Python script:

```python
import qbos as qb
tqb = qb.core()
tqb.qb12()
tqb.qn=2
tqb.acc='loopback'
tqb.xasm = True
tqb.instring = '''__qpu__ void QBCIRCUIT(qreg q) { X(q[0]); H(q[1]); Measure(q[0]); }'''
tqb.sn=16

raw_qpu_resample = '''
    {   "accs": [
    {"acc": "dqc_gen1", "url": "https://10.10.10.120:8443", "poll_secs": 6, "poll_retrys": 100, "over_request": 48, "recursive_request": true, "resample": false, "resample_above_percentage": 100}, 
    {"acc": "qdk_gen1", "url": "https://10.10.10.121:8443", "poll_secs": 6, "poll_retrys": 100, "over_request": 48, "recursive_request": true, "resample": false, "resample_above_percentage": 100}, 
    {"acc": "loopback", "url": "http://127.0.0.1:8000", "poll_secs": 1, "poll_retrys": 100, "over_request": 48, "recursive_request": true, "resample": false, "resample_above_percentage": 100}
    ]
    }
'''

json_file = open("../../qpu_config_resample.json",'w')
json_file.write(raw_qpu_resample)
json_file.close()
tqb.qpu_config = "../../qpu_config_resample.json"

tqb.run()
assert(sum([jj for jj in (tqb.out_count[0][0]).values()]) == 16)
```