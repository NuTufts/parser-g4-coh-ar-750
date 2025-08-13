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

The CENNS Event Tree contains one branch storing the class CENNSEvent. The header can be found in

    `cenns/io/CENNSEvent.hh`.

The `CENNSEvent` class stores generator info for single particle generators for now. TODO: expand this to be more general about the generated events.
The `CENNSEvent` class also stores the output of the DAQ model in the class member container

    `std::vector<CENNSChannel> fvChannel;`


The `EDepSimEvents` TTree also is a single-branch tree storing the `TG4Event` class. The header for this class can be found at

   `cenns/io/TG4Event.h`

This class keeps three member containers:

   1. `TG4PrimaryVertexContainer Primaries`: stores info about the primary particles
   2. `TG4TrajectoryContainer Trajectories`: stores particle trajectories
   3. `TG4HitSegmentDetectors SegmentDetectors`: stores energy deposit info for the sensitive detectors

## Parsers

We store parsers for various analyses in this repository.


