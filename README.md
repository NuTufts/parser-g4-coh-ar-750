# Data Parsers for COH-LAr-750 Geant4 Sim

The goal of this repository is to produce programs that perform post-processing of the COH-LAr-750 Sim.
The sim saves highly structured data meant to generically capture energy deposit locations.
It also saves trajectory information.

The programs in this repository is meant to parse this data and then produce flat data tables (saved in a ROOT tree)
that are easier for statistical analysis programs.

## Simulation Output

The output of the simulation is a ROOT file with two TTrees:

```
 TFile*		sim_output.root	
  KEY: TTree	CENNS;1	CENNS Event Tree
  KEY: TTree	EDepSimEvents;1	Energy Deposition for Simulated Events
```

The CENNS Event Tree contains two branches. The first branch stores and instance of the class CENNSEvent. The header can be found in `cenns/io/CENNSEvent.hh`. The `CENNSEvent` class stores generator info for single particle generators for now. 
(TODO: expand this to be more general about the generated events.)
The `CENNSEvent` class might also store the detected photons for each PMT (this can be toggled on and off by the sim's macro).
This information is in the  `std::vector<CENNSChannel> CENNSEvent::fvChannel` class member.

The second branch stores a vector of `cenns::io::CENNSDAQ` objects: `vector<cenns::io::CENNSDAQ>`. 
There is a DAQ class for each daq workflow defined. Typically this is just for the PMT waveforms inside the LAr volume. 
In the future, this might include a simulation of the veto panels.

For the `EDepSimEvents` TTree, there is only a single branch. 
The branch holds an instance of the `TG4Event` class. 
The header for this class can be found at `cenns/io/TG4Event.h`.

The `TG4Event` class keeps three member containers:

   1. `TG4PrimaryVertexContainer Primaries`: stores info about the primary particles
   2. `TG4TrajectoryContainer Trajectories`: stores particle trajectories
   3. `TG4HitSegmentDetectors SegmentDetectors`: stores energy deposit info for the sensitive detectors

## Parsers

We store parsers for various analyses in this repository.

  * `flatten_edep_info`: in `src/flatten_edep_info/`. This parses the `TG4HitSegmentDetectors` container and sums the energy deposited within a given  list of physical volumes. Note that these volumes must have been indicated as senstivie detector volumes in the sim's macro.


