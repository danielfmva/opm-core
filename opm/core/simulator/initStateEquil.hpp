/*
  Copyright 2014 SINTEF ICT, Applied Mathematics.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef OPM_INITSTATEEQUIL_HEADER_INCLUDED
#define OPM_INITSTATEEQUIL_HEADER_INCLUDED

#include <opm/core/simulator/EquilibrationHelpers.hpp>
#include <opm/core/simulator/BlackoilState.hpp>
#include <opm/core/props/BlackoilPropertiesInterface.hpp>
#include <opm/core/props/BlackoilPhases.hpp>
#include <opm/core/utility/RegionMapping.hpp>
#include <opm/core/utility/Units.hpp>
#include <opm/parser/eclipse/Utility/EquilWrapper.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>

#include <array>
#include <cassert>
#include <utility>
#include <vector>

/**
 * \file
 * Facilities for an ECLIPSE-style equilibration-based
 * initialisation scheme (keyword 'EQUIL').
 */
struct UnstructuredGrid;

namespace Opm
{

    /**
     * Compute initial state by an equilibration procedure.
     *
     * The following state fields are modified:
     *   pressure(),
     *   saturation(),
     *   surfacevol(),
     *   gasoilratio(),
     *   rv().
     *
     * \param[in] grid     Grid.
     * \param[in] props    Property object, pvt and capillary properties are used.
     * \param[in] deck     Simulation deck, used to obtain EQUIL and related data.
     * \param[in] gravity  Acceleration of gravity, assumed to be in Z direction.
     */
    void initStateEquil(const UnstructuredGrid& grid,
                        const BlackoilPropertiesInterface& props,
                        const Opm::DeckConstPtr deck,
                        const Opm::EclipseStateConstPtr eclipseState,
                        const double gravity,
                        BlackoilState& state);


    /**
     * Types and routines that collectively implement a basic
     * ECLIPSE-style equilibration-based initialisation scheme.
     *
     * This namespace is intentionally nested to avoid name clashes
     * with other parts of OPM.
     */
    namespace Equil {

        /**
         * Compute initial phase pressures by means of equilibration.
         *
         * This function uses the information contained in an
         * equilibration record (i.e., depths and pressurs) as well as
         * a density calculator and related data to vertically
         * integrate the phase pressure ODE
         * \f[
         * \frac{\mathrm{d}p_{\alpha}}{\mathrm{d}z} =
         * \rho_{\alpha}(z,p_{\alpha})\cdot g
         * \f]
         * in which \f$\rho_{\alpha}$ denotes the fluid density of
         * fluid phase \f$\alpha\f$, \f$p_{\alpha}\f$ is the
         * corresponding phase pressure, \f$z\f$ is the depth and
         * \f$g\f$ is the acceleration due to gravity (assumed
         * directed downwords, in the positive \f$z\f$ direction).
         *
         * \tparam Region Type of an equilibration region information
         *                base.  Typically an instance of the EquilReg
         *                class template.
         *
         * \tparam CellRange Type of cell range that demarcates the
         *                cells pertaining to the current
         *                equilibration region.  Must implement
         *                methods begin() and end() to bound the range
         *                as well as provide an inner type,
         *                const_iterator, to traverse the range.
         *
         * \param[in] G     Grid.
         * \param[in] reg   Current equilibration region.
         * \param[in] cells Range that spans the cells of the current
         *                  equilibration region.
         * \param[in] grav  Acceleration of gravity.
         *
         * \return Phase pressures, one vector for each active phase,
         * of pressure values in each cell in the current
         * equilibration region.
         */
        template <class Region, class CellRange>
        std::vector< std::vector<double> >
        phasePressures(const UnstructuredGrid& G,
                       const Region&           reg,
                       const CellRange&        cells,
                       const double            grav = unit::gravity);



        /**
         * Compute initial phase saturations by means of equilibration.
         *
         * \tparam Region Type of an equilibration region information
         *                base.  Typically an instance of the EquilReg
         *                class template.
         *
         * \tparam CellRange Type of cell range that demarcates the
         *                cells pertaining to the current
         *                equilibration region.  Must implement
         *                methods begin() and end() to bound the range
         *                as well as provide an inner type,
         *                const_iterator, to traverse the range.
         *
         * \param[in] reg             Current equilibration region.
         * \param[in] cells           Range that spans the cells of the current
         *                            equilibration region.
         * \param[in] props           Property object, needed for capillary functions.
         * \param[in] phase_pressures Phase pressures, one vector for each active phase,
         *                            of pressure values in each cell in the current
         *                            equilibration region.
         * \return                    Phase saturations, one vector for each phase, each containing
         *                            one saturation value per cell in the region.
         */
        template <class Region, class CellRange>
        std::vector< std::vector<double> >
        phaseSaturations(const Region&           reg,
                         const CellRange&        cells,
                         BlackoilPropertiesInterface& props,
                         const std::vector<double> swat_init,
                         std::vector< std::vector<double> >& phase_pressures);



        /**
         * Compute initial Rs values.
         *
         * \tparam CellRangeType Type of cell range that demarcates the
         *                cells pertaining to the current
         *                equilibration region.  Must implement
         *                methods begin() and end() to bound the range
         *                as well as provide an inner type,
         *                const_iterator, to traverse the range.
         *
         * \param[in] grid            Grid.
         * \param[in] cells           Range that spans the cells of the current
         *                            equilibration region.
         * \param[in] oil_pressure    Oil pressure for each cell in range.
         * \param[in] temperature     Temperature for each cell in range.
         * \param[in] rs_func         Rs as function of pressure and depth.
         * \return                    Rs values, one for each cell in the 'cells' range.
         */
        template <class CellRangeType>
        std::vector<double> computeRs(const UnstructuredGrid& grid,
                                      const CellRangeType& cells,
                                      const std::vector<double> oil_pressure,
                                      const std::vector<double>& temperature,
                                      const Miscibility::RsFunction& rs_func,
                                      const std::vector<double> gas_saturation);

        namespace DeckDependent {
            inline
            std::vector<EquilRecord>
            getEquil(const Opm::DeckConstPtr deck)
            {
                if (deck->hasKeyword("EQUIL")) {
                
                    Opm::EquilWrapper eql(deck->getKeyword("EQUIL"));

                    const int nrec = eql.numRegions();

                    std::vector<EquilRecord> ret;
                    ret.reserve(nrec);
                    for (int r = 0; r < nrec; ++r) {

                        EquilRecord record =
                            {
                                { eql.datumDepth(r)                        ,
                                  eql.datumDepthPressure(r)                }
                                ,
                                { eql.waterOilContactDepth(r)              ,
                                  eql.waterOilContactCapillaryPressure(r)  }
                                ,
                                { eql.gasOilContactDepth(r)                ,
                                  eql.gasOilContactCapillaryPressure(r)    }
                                ,
                                eql.liveOilInitProceedure(r)
                                ,
                                eql.wetGasInitProceedure(r)
                                ,
                                eql.initializationTargetAccuracy(r)
                            };
                        if (record.N != 0) {
                            OPM_THROW(std::domain_error,
                              "kw EQUIL, item 9: Only N=0 supported.");
                        }
                        ret.push_back(record);
                    }

                    return ret;
                }
                else {
                    OPM_THROW(std::domain_error,
                              "Deck does not provide equilibration data.");
                }
            }

            inline
            std::vector<int>
            equilnum(const Opm::DeckConstPtr deck,
                     const Opm::EclipseStateConstPtr eclipseState,
                     const UnstructuredGrid&  G   )
            {
                std::vector<int> eqlnum;
                if (deck->hasKeyword("EQLNUM")) {
                    eqlnum.resize(G.number_of_cells);                   
                    const std::vector<int>& e = 
                        eclipseState->getIntGridProperty("EQLNUM")->getData();                    
                    const int* gc = G.global_cell;
                    for (int cell = 0; cell < G.number_of_cells; ++cell) {
                        const int deck_pos = (gc == NULL) ? cell : gc[cell];
                        eqlnum[cell] = e[deck_pos] - 1;
                    }
                }
                else {
                    // No explicit equilibration region.
                    // All cells in region zero.
                    eqlnum.assign(G.number_of_cells, 0);
                }

                return eqlnum;
            }


            class InitialStateComputer {
            public:
                InitialStateComputer(BlackoilPropertiesInterface& props,
                                     const Opm::DeckConstPtr            deck,
                                     const Opm::EclipseStateConstPtr eclipseState,
                                     const UnstructuredGrid&            G    ,
                                     const double                       grav = unit::gravity)
                    : pp_(props.numPhases(),
                          std::vector<double>(G.number_of_cells)),
                      sat_(props.numPhases(),
                          std::vector<double>(G.number_of_cells)),
                      rs_(G.number_of_cells),
                      rv_(G.number_of_cells)
                {
                    // Get the equilibration records.
                    const std::vector<EquilRecord> rec = getEquil(deck);

                    // Create (inverse) region mapping.
                    const RegionMapping<> eqlmap(equilnum(deck, eclipseState, G)); 

                    // Create Rs functions.
                    rs_func_.reserve(rec.size());
                    if (deck->hasKeyword("DISGAS")) {                    
                        const std::vector<RsvdTable>& rsvdTables = eclipseState->getRsvdTables();
                        for (size_t i = 0; i < rec.size(); ++i) {
                            const int cell = *(eqlmap.cells(i).begin());                   
                            if (rec[i].live_oil_table_index > 0) {
                                if (rsvdTables.size() > 0 && size_t(rec[i].live_oil_table_index) <= rsvdTables.size()) { 
                                    rs_func_.push_back(std::make_shared<Miscibility::RsVD>(props,
                                                                                           cell,
                                                                                           rsvdTables[i].getDepthColumn(),
                                                                                           rsvdTables[i].getRsColumn()));
                                } else {
                                    OPM_THROW(std::runtime_error, "Cannot initialise: RSVD table " << (rec[i].live_oil_table_index) << " not available.");
                                }
                            } else {
                                if (rec[i].goc.depth != rec[i].main.depth) {
                                    OPM_THROW(std::runtime_error,
                                              "Cannot initialise: when no explicit RSVD table is given, \n"
                                              "datum depth must be at the gas-oil-contact. "
                                              "In EQUIL region " << (i + 1) << "  (counting from 1), this does not hold.");
                                }
                                const double p_contact = rec[i].main.press;
                                const double T_contact = 273.15 + 20; // standard temperature for now
                                rs_func_.push_back(std::make_shared<Miscibility::RsSatAtContact>(props, cell, p_contact, T_contact));
                            }
                        }
                    } else {
                        for (size_t i = 0; i < rec.size(); ++i) {
                            rs_func_.push_back(std::make_shared<Miscibility::NoMixing>());
                        }
                    }                    

                    rv_func_.reserve(rec.size());
                    if (deck->hasKeyword("VAPOIL")) {                    
                        const std::vector<RvvdTable>& rvvdTables = eclipseState->getRvvdTables();
                        for (size_t i = 0; i < rec.size(); ++i) {
                            const int cell = *(eqlmap.cells(i).begin());                   
                            if (rec[i].wet_gas_table_index > 0) {
                                if (rvvdTables.size() > 0 && size_t(rec[i].wet_gas_table_index) <= rvvdTables.size()) { 
                                    rv_func_.push_back(std::make_shared<Miscibility::RvVD>(props,
                                                                                           cell,
                                                                                           rvvdTables[i].getDepthColumn(),
                                                                                           rvvdTables[i].getRvColumn()));
                                } else {
                                    OPM_THROW(std::runtime_error, "Cannot initialise: RVVD table " << (rec[i].wet_gas_table_index) << " not available.");
                                }
                            } else {
                                if (rec[i].goc.depth != rec[i].main.depth) {
                                    OPM_THROW(std::runtime_error,
                                              "Cannot initialise: when no explicit RVVD table is given, \n"
                                              "datum depth must be at the gas-oil-contact. "
                                              "In EQUIL region " << (i + 1) << "  (counting from 1), this does not hold.");
                                }
                                const double p_contact = rec[i].main.press + rec[i].goc.press;
                                const double T_contact = 273.15 + 20; // standard temperature for now
                                rv_func_.push_back(std::make_shared<Miscibility::RvSatAtContact>(props, cell, p_contact, T_contact));
                            }
                        }
                    } else {
                        for (size_t i = 0; i < rec.size(); ++i) {
                            rv_func_.push_back(std::make_shared<Miscibility::NoMixing>());
                        }
                    }
                    

                    // Check for presence of kw SWATINIT
                    if (deck->hasKeyword("SWATINIT")) {
                        const std::vector<double>& swat_init = eclipseState->getDoubleGridProperty("SWATINIT")->getData();
                        swat_init_.resize(G.number_of_cells);
                        const int* gc = G.global_cell;
                        for (int c = 0; c < G.number_of_cells; ++c) {
                            const int deck_pos = (gc == NULL) ? c : gc[c];
                            swat_init_[c] = swat_init[deck_pos];
                        }
                    }

                    // Compute pressures, saturations, rs and rv factors.
                    calcPressSatRsRv(eqlmap, rec, props, G, grav);

                    // Modify oil pressure in no-oil regions so that the pressures of present phases can
                    // be recovered from the oil pressure and capillary relations.
                }

                typedef std::vector<double> Vec;
                typedef std::vector<Vec>    PVec; // One per phase.

                const PVec& press() const { return pp_; }
                const PVec& saturation() const { return sat_; }
                const Vec& rs() const { return rs_; }
                const Vec& rv() const { return rv_; }

            private:
                typedef DensityCalculator<BlackoilPropertiesInterface> RhoCalc;
                typedef EquilReg<RhoCalc> EqReg;

                std::vector< std::shared_ptr<Miscibility::RsFunction> > rs_func_;
                std::vector< std::shared_ptr<Miscibility::RsFunction> > rv_func_;

                PVec pp_;
                PVec sat_;
                Vec rs_;
                Vec rv_;
                Vec swat_init_;

                template <class RMap>
                void
                calcPressSatRsRv(const RMap&                       reg  ,
                                 const std::vector< EquilRecord >& rec  ,
                                 Opm::BlackoilPropertiesInterface& props,
                                 const UnstructuredGrid&           G    ,
                                 const double grav)
                {
                    for (typename RMap::RegionId
                             r = 0, nr = reg.numRegions();
                         r < nr; ++r)
                    {
                        const typename RMap::CellRange cells = reg.cells(r);

                        const int repcell = *cells.begin();
                        const RhoCalc calc(props, repcell);
                        const EqReg eqreg(rec[r], calc,
                                          rs_func_[r], rv_func_[r],
                                          props.phaseUsage());
                   
                        PVec press = phasePressures(G, eqreg, cells, grav);
                        const std::vector<double>& temp = temperature(G, eqreg, cells);

                        const PVec sat = phaseSaturations(G, eqreg, cells, props, swat_init_, press);

                        const int np = props.numPhases();
                        for (int p = 0; p < np; ++p) {
                            copyFromRegion(press[p], cells, pp_[p]);
                            copyFromRegion(sat[p], cells, sat_[p]);
                        }
                        if (props.phaseUsage().phase_used[BlackoilPhases::Liquid]
                            && props.phaseUsage().phase_used[BlackoilPhases::Vapour]) {
                            const int oilpos = props.phaseUsage().phase_pos[BlackoilPhases::Liquid];
                            const int gaspos = props.phaseUsage().phase_pos[BlackoilPhases::Vapour];
                            const Vec rs = computeRs(G, cells, press[oilpos], temp, *(rs_func_[r]), sat[gaspos]);
                            const Vec rv = computeRs(G, cells, press[gaspos], temp, *(rv_func_[r]), sat[oilpos]);
                            copyFromRegion(rs, cells, rs_);
                            copyFromRegion(rv, cells, rv_);
                        }
                    }
                }

                template <class CellRangeType>
                void copyFromRegion(const Vec& source,
                                    const CellRangeType& cells,
                                    Vec& destination)
                {
                    auto s = source.begin();
                    auto c = cells.begin();
                    const auto e = cells.end();
                    for (; c != e; ++c, ++s) {
                        destination[*c] = *s;
                    }
                }

            };
        } // namespace DeckDependent
    } // namespace Equil
} // namespace Opm

#include <opm/core/simulator/initStateEquil_impl.hpp>

#endif // OPM_INITSTATEEQUIL_HEADER_INCLUDED
