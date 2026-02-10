# izhnet

izhnet is an implementation of the **Izhikevich spiking neuron model**, a computationally efficient neuron model capable of reproducing a wide range of biologically observed spiking and bursting behaviors.

All voltages are expressed in millivolts (mV), time in milliseconds (ms), and currents in arbitrary units (with membrane resistance normalized to 1).

## State Variables

Each neuron is described by two coupled state variables:

- $v(t)$ — membrane potential
- $u(t)$ — membrane recovery variable

The recovery variable represents slow ionic currents and provides negative feedback to the membrane potential.

## Membrane Potential Dynamics

The neuron membrane dynamics are governed by the following nonlinear system:

$$\frac{dv}{dt} = 0.04 \cdot v^2 + 5 \cdot v + 140 - u + I(t)$$

$$\frac{du}{dt} = a \cdot (b \cdot v - u)$$

where $I(t)$ is the input current, which may include constant drive and synaptic input. This quadratic membrane equation enables rich spiking dynamics while remaining computationally inexpensive compared to conductance-based models.

## Spike Emission and Reset Mechanism

A spike is emitted when the membrane potential reaches a fixed threshold:

$$v \ge V_{\text{th}}$$

When this condition is met, the neuron state is instantaneously reset according to:

$$v \leftarrow c$$

$$u \leftarrow u + d$$

This reset mechanism captures spike generation, refractoriness, and adaptation without explicitly modeling ion channels.

## Synaptic Input

Incoming spikes from other neurons contribute additively to the input current:

$$I(t) \leftarrow I(t) + w$$

where $w$ is the synaptic weight associated with the presynaptic neuron.

## Numerical Integration

The model is integrated numerically using an explicit Euler method. Two integration schemes are supported:

### 1. Standard Euler Integration (Default)

Both state variables are updated using the same timestep:

$$v(t + \Delta t),\ u(t + \Delta t)$$

This method is recommended for general use and network simulations.

### 2. Published Integration Scheme

To reproduce the original results from Izhikevich (2003), the model optionally supports the published numerical method:

- The recovery variable $u$ is updated using the _new_ value of $v$.
- The membrane potential $v$ is updated using two half-steps per timestep.

This behavior is controlled by a boolean parameter:

```cpp
consistent_integration = true   # standard Euler (recommended)
consistent_integration = false  # original published scheme
```

## Model Parameters

| Parameter       | Description                    | Default            |
| :-------------- | :----------------------------- | :----------------- |
| $v$             | Membrane potential             | $-65\ \text{mV}$   |
| $u$             | Recovery variable              | $-13\ \text{mV}$   |
| $V_{\text{th}}$ | Spike threshold                | $30\ \text{mV}$    |
| $I_e$           | Constant input current         | $0$                |
| $a$             | Recovery time scale            | $0.02$             |
| $b$             | Recovery sensitivity           | $0.2$              |
| $c$             | After-spike reset potential    | $-65\ \text{mV}$   |
| $d$             | After-spike recovery increment | $8$                |
| $V_{\min}$      | Absolute lower bound on $v$    | $-1.79\ \text{mV}$ |

Different neuron types (regular spiking, fast spiking, bursting, chattering, etc.) are obtained by selecting different parameter sets $(a, b, c, d)$.

## Initial Conditions

Typical initialization at rest:

$$v(0) = -65\ \text{mV}$$

$$u(0) = b \cdot v(0)$$

## External Stimulus

The input current may be defined as a time-dependent function. A common step stimulus is:

$$I(t) = \begin{cases} A, & t_0 \le t \le t_1 \\\\ 0, & \text{otherwise} \end{cases}$$

This stimulus can induce tonic spiking, bursting, or silence depending on the chosen parameters.

## Model Characteristics

- Captures spike generation, refractoriness, and adaptation.
- Supports biologically realistic firing patterns.
- Computationally efficient for large-scale networks.
- Not conductance-based (phenomenological model).

## References

1. Izhikevich, E. M. (2003). _Simple model of spiking neurons._ IEEE Transactions on Neural Networks, 14(6), 1569–1572. [DOI: 10.1109/TNN.2003.820440](https://doi.org/10.1109/TNN.2003.820440)
2. Pauli, R., Weidel, P., Kunkel, S., & Morrison, A. (2018). _Reproducing polychronization: A guide to maximizing the reproducibility of spiking network models._ Frontiers in Neuroinformatics, 12. [DOI: 10.3389/fninf.2018.00046](https://doi.org/10.3389/fninf.2018.00046)
3. NEST Simulator Documentation. _Izhikevich model (v3.9)._ https://nest-simulator.readthedocs.io/en/v3.9/models/izhikevich.html
4. SETI Neuron Lab. _Izhikevich Model and backpropagation (PDF)._ https://www.seti.net/Neuron%20Lab/NeuronReferences/Izhikevich%20Model%20and%20backpropagation.pdf
5. Nengo Documentation. _Izhikevich example (v3.0.0)._ https://www.nengo.ai/nengo//v3.0.0/examples/advanced/izhikevich.html
6. Musacchio, F. (2024). _Izhikevich network model._ https://www.fabriziomusacchio.com/blog/2024-05-19-izhikevich_network_model/
7. Emergent Mind. _Izhikevich neurons topic._ https://www.emergentmind.com/topics/izhikevich-neurons
