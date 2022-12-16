#include "readData.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <chrono>
#include <cfloat>
#include <cmath>
#include <tuple>
#include <algorithm>
#include <iomanip>

using namespace std;

int dimension; //Number of locations
double ** distance_matrix; //Matrix representing the distance between two cities.

struct insertion{
  /*
  *This insertion structure helps representing the node insertion into a given solution s'.
  *
  *@param node Integer which represents the evaluated node to be inserted into the solution s'.
  *@param deleted_edge Index of the removed edge when inserting the new node.
  *@param cost Cost of the insertion movement
  */
  int node; 
  tuple<int,int> deleted_edge; //
  double cost; //
};

struct solution{
  /*
  *Structure representing a route solution with its cost.
  *
  *@param route Vector representing a sequence of nodes starting from the first node, visiting all the remaining and returning to the depot.
  *@param accumulated_cost Total accumulated cost from depot to the last node in the route.
  */

  vector<int> route; //
  double cumulated_cost;
};

struct Subsequence{
  
  double T, C;
  int W;
  int first, last;

  inline static Subsequence Concatenate(Subsequence &sigma1, Subsequence &sigma2){
    /*
    * Inline function which returns a subsequence route and its first and last nodes, cumulative and delay costs and timeframe. 
    * All of them represented in the parameters below.
    *
    * @param sigma1 First subsequence of a given solution s to a MLP instance.
    * @param sigma2 Second subsequence of a given solution s to a MLP instance.
    * @param C Cumulative cost of a subsequence sigma.
    * @param W Delay cost of a subsequence sigma. (auxiliary)
    * @param T Timeframe of a subsequence sigma. (auxiliary)
    * 
    * ----------------------------------------------------------
    * 
    * @return sigma Subsequence consisted of sigma1 and sigma2.
    */
    Subsequence sigma;

    double temp = distance_matrix[sigma1.last][sigma2.first];
    sigma.W = sigma1.W + sigma2.W;
    sigma.T = sigma1.T + temp + sigma2.T;
    sigma.C = sigma1.C + sigma2.W * (sigma1.T + temp) + sigma2.C;
    sigma.first = sigma1.first; 
    sigma.last = sigma2.last;

    return sigma;
  }
};

bool compares(insertion& insertion1, insertion& insertion2){
    /*
    * Compares the cost between two insertions.
    *
    *@param insertion1 First insertion structure.
    *@param insertion2 Second insertion structure.
    *
    *----------------------------------------------------------
    *
    *@return True if the cost of insertion2 is higher than insertion1.
    */
    return insertion1.cost < insertion2.cost;
  }

vector<insertion> calculate_insertion_cost(int origin, vector<int>& CL_){
  /*
  * Generates a group of candidate nodes and their insertion cost if connected to the origin node in the route.
  * 
  * @param origin A given node in the route.
  * @param CL_ List of candidates to be inserted in the route.
  * 
  * ----------------------------------------------------------
  * @return insertion_cost Structure composed by the candidate nodes and their insertion cost from the origin node.  
  * 
  * */
  //generates a full list of edges between s' nodes and the nodes from candidates list
  
  vector<insertion> insertion_cost(CL_.size());

  for(int i = 0; i < CL_.size(); i++){
    insertion_cost[i].cost = distance_matrix[origin][CL_[i]];
    insertion_cost[i].node = CL_[i];
  }

  return insertion_cost;
}

solution construction(vector<int> CL_) {
  /*
  * Creates an initial random route solution for the problem.
  * 
  * @param CL_ List of candidates to be inserted in the route.
  * 
  * ----------------------------------------------------------
  * 
  * @return s_ Initial solution composed by the route and its cumulated cost. 
  * */

  int random_index, j, i, origin = 1;
  solution s_;
  
  //Add the node 1 as the depot in the route
  s_.route.push_back(1);

  vector<insertion> insertion_cost;

  int range;
  double alpha= (rand() % 26) / 100.0;

  while(!CL_.empty()){

    insertion_cost = calculate_insertion_cost(origin, CL_);
    
    sort(insertion_cost.begin(), insertion_cost.end(), compares);
    
    range= (int) floor(alpha * insertion_cost.size());

    int selected_node;

    if(range != 0){
     selected_node = rand() % range;
    }else{
      selected_node = range;
    }
    
    origin = insertion_cost[selected_node].node;
    
    j=0;
    while(1){
      if(origin == CL_[j]){
      
        CL_.erase(CL_.begin()+j);
        break;
      
      }
      j++;
    }
    s_.route.push_back(origin);
    
  }   
  s_.route.push_back(1);
  return s_;
}

void update_all_subsequences(solution s, vector<vector<Subsequence>> &subseq_matrix){
  /*
  * Updates all subquences values of a given solution s.
  *
  * ----------------------------------------------------------
  * 
  * @param s Current solution before a new movement is aplied if subseq_matrix[i][j].C is less than the current cumulated cost of s. 
  * @param subseq_matrix Stores all the information specified in the Subsequence structure between two nodes in the route. Example: subseq_matrix[i][j] stores all the 
  * information between nodes i and j - subseq_matrix[i][j].C is the cumulated cost from i to j. 
  * 
  */
  

  int n = s.route.size();

  for (int i = 0; i < n; i++){

    int v = s.route[i];
    
    subseq_matrix[i][i].W = (i > 0);
    subseq_matrix[i][i].C = 0;
    subseq_matrix[i][i].T = 0;
    subseq_matrix[i][i].first = s.route[i];
    subseq_matrix[i][i].last = s.route[i];
  }

  for (int i = 0; i < n; i++){
    for (int j = i + 1; j < n; j++){
      subseq_matrix[i][j] = Subsequence::Concatenate(subseq_matrix[i][j-1], subseq_matrix[j][j]);
    }
  }
  
  for (int i = n - 1; i >= 0; i--){
    for (int j = i - 1; j >= 0; j--){
      subseq_matrix[i][j] = Subsequence::Concatenate(subseq_matrix[i][j+1], subseq_matrix[j][j]);
    }  
  }

}

bool apply_swap_movement(solution& s_, vector<vector<Subsequence>> &subseq_matrix){
  /*
  * Evaluates and applies a swap between two nodes i and j if that change reduces the entire route cumulated cost compared to the current solution.  
  *
  * @param s_ Current solution before applying the swap movement.
  * @param subseq_matrix Stores all the information specified in the Subsequence structure between two nodes in the route s_.
  * 
  * ----------------------------------------------------------
  * 
  * @return bool True if swap movement reduced the entire route cumulated cost. Otherwise, False.
  * 
  */

  //Cumulated cost of current solution
  double best_cost = subseq_matrix[0][dimension].C;
  int best_i, best_j;

  //Auxialiry structures to store subsequences' informations before and after i and j nodes when evaluating the swap movement.
  Subsequence s1, s2, s3, s4, s5;
  

  for(int i =1; i< s_.route.size() - 1; i++){

    for(int j=i+1; j< s_.route.size() - 1; j++){
      s1 = Subsequence::Concatenate(subseq_matrix[0][i-1],subseq_matrix[j][j]);

      if(i==j-1){
        s2 = Subsequence::Concatenate(s1,subseq_matrix[i][i]); //swap
        s4 = Subsequence::Concatenate(s2,subseq_matrix[j+1][dimension]);
      }
      else{
        s2 = Subsequence::Concatenate(s1, subseq_matrix[i+1][j-1]);
        s3 = Subsequence::Concatenate(s2, subseq_matrix[i][i]);
        s4 = Subsequence::Concatenate(s3, subseq_matrix[j+1][dimension]);
      }

      if(s4.C < best_cost){
        best_cost = s4.C;
        best_i = i;
        best_j = j;
      }
    }
  }
  
  if(best_cost < subseq_matrix[0][dimension].C){
    //Apply swap movement and updates all subsequences if the swap movement should be adopted.

    swap(s_.route[best_i], s_.route[best_j]);
    update_all_subsequences(s_, subseq_matrix);
    return true;
  }
  return false;
}

bool apply_2opt_movement(solution& s_, vector<vector<Subsequence>> &subseq_matrix){
  /*
  * Evaluates and applies a 2-opt movement between two nodes i and j if that change reduces the entire route cumulated cost compared to the current solution.  
  *
  * @param s_ Current solution before applying the 2opt movement.
  * @param subseq_matrix Stores all the information specified in the Subsequence structure between two nodes in the route s_.
  * 
  * ----------------------------------------------------------
  * 
  * @return bool True if 2opt movement reduced the entire route cumulated cost. Otherwise, False.
  * 
  */

  int best_i, best_j, i, j;
  double best_cost = subseq_matrix[0][dimension].C;

  Subsequence s1, s2;

  for(int i = 1; i< s_.route.size() - 2; i++){
    
    //a = distance_matrix[s_.route[i-1]][s_.route[i]];
    //partial_cost = - a;

    for(int j = i + 2; j< s_.route.size() - 1; j++){
      
      s1 = Subsequence::Concatenate(subseq_matrix[0][i-1], subseq_matrix[j][i]);
      s2 = Subsequence::Concatenate(s1, subseq_matrix[j+1][dimension]);
     
     
  if(s2.C < best_cost){
    best_cost = s2.C;
    best_i = i;
    best_j = j;
    }
  }
}
if(best_cost < subseq_matrix[0][dimension].C){
  j=best_j;
  for(i=best_i; i<j;i++){
    swap(s_.route[i], s_.route[j]);
    j--;
  }
  update_all_subsequences(s_, subseq_matrix);
  return true;
  }
  return false;
}

bool apply_reinsertion_movement(solution& s_, vector<vector<Subsequence>> &subseq_matrix, int n){
  /*
  * Evaluates and applies a reinsertion of n nodes between two nodes i and i+1 if that change reduces the entire route cumulated cost compared to the current solution.  
  *
  * @param s_ Current solution before applying the swap movement.
  * @param subseq_matrix Stores all the information specified in the Subsequence structure between two nodes in the route s_.
  * @param n Factor to invoke any of the 3 reinsertion cases. It represents how many nodes (1, 2 or 3) after j should be inserted between i and i+1. 
  * ----------------------------------------------------------
  * 
  * @return bool True if the reinsertion movement reduces the entire route cumulated cost. Otherwise, False.
  */
  
  int i, j, best_i, best_j, index;
  double best_cost = subseq_matrix[0][dimension].C;

  Subsequence s1, s2, s3;

  switch(n){
    case 1:
      for(i=1; i< s_.route.size()-1; i++){
        for(j=1; j< s_.route.size()-1; j++){
          if(i<j){ 
            s1 = Subsequence::Concatenate(subseq_matrix[0][i-1], subseq_matrix[i+1][j]);
            s2 = Subsequence::Concatenate(s1, subseq_matrix[i][i]);
            s3 = Subsequence::Concatenate(s2, subseq_matrix[j+1][s_.route.size()-1]);
          }
          else if(j<i){
            s1 = Subsequence::Concatenate(subseq_matrix[0][j-1], subseq_matrix[i][i]);
            s2 = Subsequence::Concatenate(s1, subseq_matrix[j][i-1]);
            s3 = Subsequence::Concatenate(s2, subseq_matrix[i+1][s_.route.size()-1]);
          }
          else{
            continue;
          }
          if(s3.C < best_cost){
            best_i = i;
            best_j = j;
            best_cost = s3.C;
          }
        }
      }

      if(best_cost < subseq_matrix[0][dimension].C){
        if(best_i < best_j){
          s_.route.insert(s_.route.begin() + best_j + 1, s_.route[best_i]);
          s_.route.erase(s_.route.begin() + best_i);
        }
        else{
          s_.route.insert(s_.route.begin() + best_j, s_.route[best_i]);
          s_.route.erase(s_.route.begin()+ best_i + 1);
        }

        update_all_subsequences(s_, subseq_matrix);
        
        return true;
      }
      return false;
    case 2:
      for(i=1; i < dimension - 2; i++){
        for(j=1;j < dimension - 1; j++){
          if(i < j){
            s1 = Subsequence::Concatenate(subseq_matrix[0][i-1], subseq_matrix[i+2][j]);
            s2 = Subsequence::Concatenate(s1, subseq_matrix[i][i+1]);
            s3 = Subsequence::Concatenate(s2, subseq_matrix[j+1][s_.route.size()-1]);
          }
          else if(i > j){
            s1 = Subsequence::Concatenate(subseq_matrix[0][j-1], subseq_matrix[i][i+1]);
            s2 = Subsequence::Concatenate(s1, subseq_matrix[j][i-1]);
            s3 = Subsequence::Concatenate(s2, subseq_matrix[i+2][s_.route.size()-1]);
          }
          else{
            continue;
          }

          if(s3.C < best_cost){
            best_i = i;
            best_j = j;
            best_cost = s3.C;
          }
        }
      }
      
      if(best_cost < subseq_matrix[0][dimension].C){
        if(best_i < best_j){
          for(index = 0; index < 2; index++){
            s_.route.insert(s_.route.begin() + best_j + index + 1, s_.route[best_i + index]);
          }
          for(index = 1; index >= 0; index--){
            s_.route.erase(s_.route.begin() + best_i + index);
          }
        }
        else{
          s_.route.insert(s_.route.begin() + best_j, s_.route[best_i]);
          s_.route.insert(s_.route.begin() + best_j + 1, s_.route[best_i + 2]);
          for(index = 1; index >= 0; index--){
            s_.route.erase(s_.route.begin() + best_i + 2);
          }
        }
        
        update_all_subsequences(s_, subseq_matrix);
        
        return true;
      }
      return false;
    case 3:
      for(i=1; i < dimension - 3; i++){
        for(j=1;j < dimension - 1; j++){
          if(i < j){
            s1 = Subsequence::Concatenate(subseq_matrix[0][i-1], subseq_matrix[i+3][j]);
            s2 = Subsequence::Concatenate(s1, subseq_matrix[i][i+2]);
            s3 = Subsequence::Concatenate(s2, subseq_matrix[j+1][s_.route.size()-1]);
          }
          else if(i > j){
            s1 = Subsequence::Concatenate(subseq_matrix[0][j-1], subseq_matrix[i][i+3]);
            s2 = Subsequence::Concatenate(s1, subseq_matrix[j][i-1]);
            s3 = Subsequence::Concatenate(s2, subseq_matrix[i+3][s_.route.size()-1]);
          }
          else{
            continue;
          }
          
          if(s3.C < best_cost){
            best_i = i;
            best_j = j;
            best_cost = s3.C;
          }
        }
      }
      
      if(best_cost < subseq_matrix[0][dimension].C){
        if(best_i < best_j){
          for(index = 0; index < 3; index++){
            s_.route.insert(s_.route.begin() + best_j + index + 1, s_.route[best_i + index]);
          }
          for(index = 2; index >= 0; index--){
            s_.route.erase(s_.route.begin() + best_i + index);
          }
        }
        else{
          s_.route.insert(s_.route.begin() + best_j, s_.route[best_i]);

          for(index = 1; index <= 2; index++){
            s_.route.insert(s_.route.begin() + best_j + index, s_.route[best_i + pow(2,index)]);
          }
          for(index = 0; index < 3; index++){
            s_.route.erase(s_.route.begin() + best_i + 3);
          }
        }
        
        update_all_subsequences(s_, subseq_matrix);
        
        return true;
      }
      return false;
    }

    return false;
}

void local_search(solution& s_, vector<vector<Subsequence>> &subseq_matrix){
  
  std::vector<int> NL = {1,2,3,4,5};
  bool improved = false;
  
  while(!NL.empty()){
    int n = rand() % NL.size();
    //cout << n <<"\n n";
    switch (NL[n]){
      case 0:
        //improved = false;
        improved = apply_swap_movement(s_, subseq_matrix);
        break;
      case 1:
        //improved = false;
        improved = apply_2opt_movement(s_, subseq_matrix);
        break;
      case 2:
        improved = apply_reinsertion_movement(s_, subseq_matrix, 1); // Reinsertion
        //improved = false;
        break;
      case 3:
        //improved = false;
        improved = apply_reinsertion_movement(s_, subseq_matrix, 2); // Or-opt2
        break;
      case 4:
        //improved = false;
        improved = apply_reinsertion_movement(s_, subseq_matrix, 3); // Or-opt3
        break;
    }
    
    if (improved){
      NL = {1,2,3,4,5};
    }
    else{
      NL.erase(NL.begin() + n);
    }
  }
}

solution pertubation(solution s_, vector<vector<Subsequence>> &subseq_matrix){
  int bridge1_size, bridge2_size, i, j, max_i, min_j, max_j, j_range, large, small, index1, index2;
  vector<int> bridge1, bridge2;
  double new_route_cost = 0;
  bool changed = false;

  if(s_.route.size()<30){
    bridge1_size = rand() % 2 + 1; // rand % range + min
    bridge2_size = rand() % 2 + 1; // rand % range + min
  }
  else{
    bridge1_size = rand() % s_.route.size()/5 + 2;
    bridge2_size = rand() % s_.route.size()/5 + 2;

    if(bridge1_size>bridge2_size){
      large = bridge1_size;
      small = bridge2_size;
      changed = true;
    }
    else{
      large = bridge2_size;
      small = bridge1_size;
    }
  }

  while (1)
  {
    index1 = rand() % dimension + 1;

    if(index1 <= dimension - bridge1_size - bridge2_size){
      break;
    }
  }

  while (1)
  {
    index2 = rand() % dimension + (index1 + bridge1_size);

    if(index2 <= dimension - bridge2_size){
      break;
    }
  }
  
  for(i=0; i < small; i++){
    swap(s_.route[index1+i], s_.route[index2+i]);
  }
  
  if(bridge1_size != bridge2_size){
    int diff = large - small;
    i = 0;

    if(changed){
      while (diff)
      {
        s_.route.insert(s_.route.begin() + index2 + bridge2_size, s_.route[index1 + bridge2_size]);
        s_.route.erase(s_.route.begin() + index1 + bridge2_size);
        diff--;
        i++;
      }
    }
    else{
      while(diff){
        s_.route.insert(s_.route.begin() + index1 + bridge1_size + i, s_.route[index2 + bridge1_size + i]);
        s_.route.erase(s_.route.begin() + index2 + bridge1_size + i + 1);
        diff--;
        i++;
      }
    }
  }

  update_all_subsequences(s_, subseq_matrix);
  return s_;
}

int main(int argc, char** argv) {

  srand(time(NULL));

  auto start = chrono::steady_clock::now();

  vector<int> CL_;
  int i, max_i, max_iter_ils, count;
  double best_cum_cost, best_all_cum_cost;
  best_all_cum_cost = (double) INFINITY;
  solution s_, best_s, best_all_s;

  readData(argc, argv, &dimension, &distance_matrix);

  for(i= 2; i <= dimension; i++){ 
      CL_.push_back(i);
    }

  if(dimension>100){
    max_iter_ils = 100;
  }
  else{
    max_iter_ils = dimension;
  }

  max_i = 10;
  best_all_s.cumulated_cost = (double) INFINITY;

  for(i=0; i< max_i; i++){
    
    s_.route.clear();

    vector<vector<Subsequence>> subseq_matrix(dimension+1, vector<Subsequence>(dimension+1));

    s_ = construction(CL_);

    update_all_subsequences(s_, subseq_matrix);
  
    best_cum_cost = subseq_matrix[0][dimension].C;
    best_s = s_;

    int count = 0;
    while(count < max_iter_ils){
      local_search(s_, subseq_matrix);

      if(subseq_matrix[0][dimension].C < best_cum_cost){
        best_all_s = s_;
        best_cum_cost = subseq_matrix[0][dimension].C;
        //update_all_subsequences(s_,subseq_matrix);
        //cout << best_cum_cost << endl;
        //cout << subseq_matrix[0][dimension].C << "\n";
        count = 0;
      }

      s_ = pertubation(best_s, subseq_matrix);

      count++;
    }
    if(best_cum_cost < best_all_cum_cost){
      //update_all_subsequences(s_,subseq_matrix);
      best_all_s = best_s;
      best_all_cum_cost = best_cum_cost;
    }
  //cout << subseq_matrix[0][1].C << endl;
  }
  cout << "Best Cost: " << best_all_cum_cost << endl;
  cout << "Best Route: " << endl;
  for(auto k: best_all_s.route){
    cout << k << " ";
  }
  cout << endl;
  return 0;
}
