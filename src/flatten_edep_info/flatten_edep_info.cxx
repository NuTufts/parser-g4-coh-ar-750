#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream>
#include <sstream>

// ROOT includes
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"

// CENNS includes - from cenns_io library
#include "cenns/io/CENNSEvent.hh"
#include "cenns/io/CENNSDAQ.hh"
#include "cenns/io/TG4Event.h"
#include "cenns/io/TG4HitSegment.h"
#include "cenns/io/TG4PrimaryVertex.h"

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " <input_file> <output_file> [volumes_file]\n";
    std::cout << "  input_file:   ROOT file from g4-coh-ar-750 simulation\n";
    std::cout << "  output_file:  Output ROOT file with flattened data\n";
    std::cout << "  volumes_file: Optional text file with volume names (one per line)\n";
    std::cout << "\nExample:\n";
    std::cout << "  " << program_name << " sim_output.root flat_output.root volumes.txt\n";
    std::cout << "\nFormat of volumes.txt:\n";
    std::cout << "  LArVol\n";
    std::cout << "  volCryostat\n";
    std::cout << "  volPanel\n";
    std::cout << "  # Comments starting with # are ignored\n";
}

std::vector<std::string> read_volume_list(const std::string& filename) {
    std::vector<std::string> volumes;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Warning: Cannot open volumes file " << filename << "\n";
        std::cerr << "         Proceeding without volume filtering\n";
        return volumes;
    }
    
    std::string line;
    int line_number = 0;
    
    while (std::getline(file, line)) {
        line_number++;
        
        // Remove leading/trailing whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        volumes.push_back(line);
        std::cout << "  Line " << line_number << ": " << line << "\n";
    }
    
    file.close();
    return volumes;
}

int main( int nargs, char** argv )
{
    std::cout << "========================================\n";
    std::cout << "     Flatten Edep Info Parser\n";
    std::cout << "========================================\n";

    // Parse command line arguments
    if (nargs < 3) {
        std::cerr << "Error: Insufficient arguments\n\n";
        print_usage(argv[0]);
        return 1;
    }

    std::string input_filename = argv[1];
    std::string output_filename = argv[2];
    
    // Read volume names from file if provided
    std::vector<std::string> volume_names;
    if (nargs >= 4) {
        std::string volumes_filename = argv[3];
        std::cout << "Reading volume names from: " << volumes_filename << "\n";
        volume_names = read_volume_list(volumes_filename);
        std::cout << "Found " << volume_names.size() << " volume(s)\n";
    } else {
        std::cout << "No volumes file provided - will extract channel and primary info only\n";
    }

    std::cout << "Input file:  " << input_filename << "\n";
    std::cout << "Output file: " << output_filename << "\n";
    if (!volume_names.empty()) {
        std::cout << "Volumes to extract:\n";
        for (const auto& vol : volume_names) {
            std::cout << "  - " << vol << "\n";
        }
    }
    std::cout << "========================================\n";

    // Open the input ROOT file
    TFile* input_file = TFile::Open(input_filename.c_str(), "READ");
    if (!input_file || input_file->IsZombie()) {
        std::cerr << "Error: Cannot open input file " << input_filename << "\n";
        return 1;
    }

    // Get the two TTrees from the input file
    TTree* cenns_tree = (TTree*)input_file->Get("CENNS");
    TTree* edepsim_tree = (TTree*)input_file->Get("EDepSimEvents");

    if (!cenns_tree) {
        std::cerr << "Error: Cannot find CENNS tree in input file\n";
        input_file->Close();
        return 1;
    }
    if (!edepsim_tree) {
        std::cerr << "Error: Cannot find EDepSimEvents tree in input file\n";
        input_file->Close();
        return 1;
    }

    // Set up branches to read the data
    CENNSEvent* cenns_event = 0;
    cenns::io::TG4Event* tg4_event = 0;
    std::vector< cenns::io::CENNSDAQ >* cenns_daq_v = 0;
    
    cenns_tree->SetBranchAddress("Event",   &cenns_event);
    cenns_tree->SetBranchAddress("DAQ",     &cenns_daq_v);
    edepsim_tree->SetBranchAddress("Event", &tg4_event);

    // Check that both trees have the same number of entries
    Long64_t n_cenns_entries = cenns_tree->GetEntries();
    Long64_t n_edep_entries = edepsim_tree->GetEntries();
    
    if (n_cenns_entries != n_edep_entries) {
        std::cerr << "Warning: Different number of entries in trees (CENNS: " 
                  << n_cenns_entries << ", EDepSim: " << n_edep_entries << ")\n";
    }
    
    Long64_t n_entries = std::min(n_cenns_entries, n_edep_entries);
    std::cout << "Processing " << n_entries << " events...\n";

    // Create output file and tree
    TFile* output_file = TFile::Open(output_filename.c_str(), "RECREATE");
    if (!output_file || output_file->IsZombie()) {
        std::cerr << "Error: Cannot create output file " << output_filename << "\n";
        input_file->Close();
        return 1;
    }

    TTree* output_tree = new TTree("EdepInfo", "Flattened Energy Deposition Information");

    // Variables for output tree
    Int_t event_id;
    Double_t total_primary_energy;
    std::map<std::string, Double_t> volume_edep;
    std::vector<Double_t> channel_integrals;
    Int_t n_channels;
    Double_t all_channel_integral = 0;
    
    // Create branches for output tree
    output_tree->Branch("event_id", &event_id, "event_id/I");
    output_tree->Branch("total_primary_energy", &total_primary_energy, "total_primary_energy/D");
    output_tree->Branch("n_channels", &n_channels, "n_channels/I");
    output_tree->Branch("all_channel_integral", &all_channel_integral, "all_channel_integral/D");
    output_tree->Branch("channel_integrals", &channel_integrals);
    
    // Create branches for each requested volume
    for (const auto& vol_name : volume_names) {
        volume_edep[vol_name] = 0.0;
        std::string branch_name = "edep_" + vol_name;
        output_tree->Branch(branch_name.c_str(), &volume_edep[vol_name], 
                          (branch_name + "/D").c_str());
    }

    // Main event loop
    for (Long64_t i = 0; i < n_entries; ++i) {

        // if (i % 100 == 0 ) {
        //     std::cout << "Processing event " << i << "/" << n_entries << "\r" << std::flush;
        // }

        std::cout << "Processing event " << i << "/" << n_entries << "\r" << std::endl;

        // Read both trees for this event
        cenns_tree->GetEntry(i);
        edepsim_tree->GetEntry(i);

        // Reset variables
        event_id = i;
        total_primary_energy = 0.0;
        for (auto& kv : volume_edep) {
            kv.second = 0.0;
        }
        channel_integrals.clear();
        all_channel_integral = 0.0;

        // Extract primary particle energy from TG4Event
        if (tg4_event) {
            for (const auto& primary_vertex : tg4_event->Primaries) {
                for (const auto& particle : primary_vertex.Particles) {
                    // Energy is stored as momentum[3] in TG4PrimaryParticle
                    total_primary_energy += particle.Momentum[3]; // E component
                }
            }
        }

        // Extract energy deposits for specified volumes from TG4Event
        if (tg4_event && !volume_names.empty()) {
            // Loop through segment detectors
            for (const auto& [det_name, segments] : tg4_event->SegmentDetectors) {
                std::cout << "Loop through [" << det_name << "] hits" << std::endl;
                // Check if this detector name matches any requested volume
                // for (const auto& vol_name : volume_names) {
                //     if (det_name.find(vol_name) != std::string::npos) {
                //         // Sum energy deposits in this volume
                //         for (const auto& segment : segments) {
                //             volume_edep[vol_name] += segment.EnergyDeposit;
                //         }
                //     }
                // }

                for (const auto& segment : segments) {

                    // for debug: uncomment line below to get dump of volume names of edep segments
		    // std::cout << "  [" << segment.PVname << ":" << segment.VolIDname << "] " << segment.GetEnergyDeposit() << std::endl;
		    
                    auto it_voledep = volume_edep.find(segment.PVname);
                    if ( it_voledep!=volume_edep.end()) {
                        it_voledep->second += segment.GetEnergyDeposit();
                    }
                }
            }
        }

        // Extract channel waveform integrals from CENNSEvent
        if (cenns_daq_v) {

            for (auto const& daqdata : *cenns_daq_v ) {
                int nwaveforms = daqdata.waveforms.size();
                std::cout << "  number of waveforms: " << nwaveforms << std::endl;

                if (nwaveforms>n_channels) {
                    n_channels = nwaveforms;
                }
                channel_integrals.resize(n_channels,0);
            
                for (const auto& wfm : daqdata.waveforms ) {
                    // Calculate waveform integral
                    Double_t integral = 0.0;
                    int chid = wfm.chid;
                    if (wfm.size() > 0) {
                        for (auto const& sample : wfm) {
                            integral += sample;
                        }
                        // Multiply by time bin width if needed
                        integral *= wfm.sample_period;
                    }
                    if ( chid>=n_channels ) {
                        n_channels = chid;
                        channel_integrals.resize(n_channels,0);
                    }
                    channel_integrals[chid] = integral;
                    all_channel_integral += integral;
                }
            }
        }
        else {
            n_channels = 0;
        }

        // Fill the output tree
        output_tree->Fill();
    }

    std::cout << "\nProcessing complete!\n";

    // Write and close files
    output_file->cd();
    output_tree->Write();
    
    // Print summary
    std::cout << "\nSummary:\n";
    std::cout << "  Events processed: " << n_entries << "\n";
    std::cout << "  Output tree entries: " << output_tree->GetEntries() << "\n";
    std::cout << "  Output file: " << output_filename << "\n";
    
    output_file->Close();
    input_file->Close();

    delete output_file;
    delete input_file;

    std::cout << "Done!\n";
    
    return 0;
}
