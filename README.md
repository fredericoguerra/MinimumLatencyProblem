# Minimum Latency Problem (MLP)
This repository contains the implementation of a metaheuristic called GILS-RVND which combines GRASP, ILS and RVND for the Minimum Latency Problem (MLP).

## Running the project
From the project folder, open your terminal and execute the following commands:

```
./mlp instances/<INSTANCE_NAME>
```

All the instances for testing are available in the [instances](https://github.com/fredericoguerra/TravelingSalesmanProblem/tree/main/instances) folder under this project.


## Running the notebook in a local virtual environment on ubuntu

Make sure you have Python3.8 installed locally, then you should run the following codes to install the venv package, create and activate a virtual environment named mlpenv.

```
apt install python3.8-venv

python3 -m venv <directory-for-virtual-env>

python3 -m venv mlpenv

source mlpenv/bin/activate

```

Install jupyter lab and initiate a session:

```
pip install jupyterlab

apt install pipenv

pipenv run jupyter lab

```