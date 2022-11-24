# Ising Model
Interactive real-time simulation of the 2D Ising model with OpenGl.

Let's consider a 2D NxN spin lattice, where each spin can assume the values $\sigma_i=\pm 1$. The total system Hamiltonian, by further assuming
  -uniform interaction term $J_{ij}=J$ for nearest neighbours (n.n.), 0 otherwise.
  -uniform external magnetic field $h_i=h$
is then:

$H = -J\sum_{i,j=n.n} \sigma_i\sigma_j - \mu h \sum_{i} \sigma_i$

where $\mu$ is the magnetic moment. For electrons we assume for convention $\mu = |\mu_e| = -\mu_e$, so that the negative sign is written explicitly in the Hamiltonian.

For the 2D case we have 4 nearest neighbours, let's call them up (U), down (D), left(L) and (R) right. Then the Hamiltonian at a local site $i$ can be rewritten as:

$H_i = - \sigma_i \left(J  (\sigma_U(i) + \sigma_D(i) + \sigma_L(i) + \sigma_R(i) )  + \mu h \right)$

For simplicity we can re-parameterize the external magnetic field $\bar h := \mu h / J$, in this way the equation simplifies as:

$H_i = -J \sigma_i \left(\sigma_U(i) + \sigma_D(i) + \sigma_L(i) + \sigma_R(i)  + \bar h \right)$


So a single spin flip at the site $i$, that is $\sigma_{i, new} = - \sigma_{i, old}$ produces a change in the energy of the system 
$\Delta H_i = H_{i, new} - H_{i, old} = -J (\sigma_{i, new} - \sigma_{i, old}) \left(\sigma_U(i) + \sigma_D(i) + \sigma_L(i) + \sigma_R(i)  + \bar h \right)$

Since $(\sigma_{i, new} - \sigma_{i, old}) = -2\sigma_{i, old} $, we have (dropping the index old, meaning that we are calculating the variation of the energy that we induce by flipping the spin at site $i$ from its current value $\sigma_i$:
$\Delta H_i = +2J \sigma_i \left(\sigma_U(i) + \sigma_D(i) + \sigma_L(i) + \sigma_R(i)  + \bar h \right)$

In the Metropolis-Hastings algorithm we are accepting the spin flip with acceptance rate

$A = \min \left(exp\left(\frac{-\Delta H_i}{k_b T}\right) ,1\right)$


The value of the critical temperature from Onsager's exact solution for the 2D model is 

$T_c = \frac{2|J|}{k_b \ln(1+\sqrt{2})}$


For our simulation, instead of the absolute temperature $T$ we use as a parameter the more significant ratio $R_T := T/T_c$, so we can rewrite $k_b T$ as a function of this parameter, and by also introducing the constant $\bar{T}_c = k_b T_c := \frac{2|J|}{\ln(1+\sqrt{2})}$ we obtain:

$k_b T \equiv  R_T k_b T_c = R_T \bar{T}_c$

completely removing the model's dependence on $k_b$.

To summarize, the model depends on two external variables, $R_T := T/T_c$ and $\bar h := \mu h / J$, with acceptance rate

$A = \min \left(exp\left(\frac{-\Delta H_i}{R_T \bar{T}_c}\right) ,1\right)$

In the program you can choose the initial distribution of the spins (random, all up, all down) and the value of J:

(J=1 -> ferromagnetic, J=-1 -> antiferromagnetic) 

