#include <fantom/algorithm.hpp>
#include <fantom/register.hpp>
#include <fantom/graphics.hpp>
#include <fantom/fields.hpp>

using namespace fantom;

namespace {

	class ShowGrid : public VisAlgorithm {

		std::unique_ptr< Primitive > m_gridLines;

	public:
		static const bool isAutoRun = false;

		struct Options : public VisAlgorithm::Options {
			Options( fantom::Options::Control& control ) :
				VisAlgorithm::Options( control )
			{
				add< Grid< 3 > >( "Grid", "Grid that will be visualized" );
				add< Color >( "Color", "Grid color", Color( 0.0, 0.0, 1.0 ) );
			}
		};

		struct VisOutputs : public VisAlgorithm::VisOutputs {
			VisOutputs( fantom::VisOutputs::Control& control ) :
				VisAlgorithm::VisOutputs( control )
			{
				addGraphics( "grid" );
			}
		};

		ShowGrid( InitData& data ) :
			VisAlgorithm( data )
		{

		}

		virtual void execute( const Algorithm::Options& options, const volatile bool& abortFlag ) override {
			m_gridLines = getGraphics( "grid" ).makePrimitive();

			std::shared_ptr< const Grid< 3 > > grid = options.get< Grid< 3 > >( "Grid" );
			Color color = options.get< Color >( "Color" );

			if( !grid ) {
				infoLog() << "Input grid not set!" << std::endl;
				return;
			}

			int numCells = grid->numCells();
			int gridDimension = grid->getMaximalCellDimension();	// assuming grid is structured!!

			const ValueArray< Point3 >& points = grid->points();
			std::vector< Vector3 > vertices( numCells * 12 * 2 );

			#pragma omp parallel for 
			for (int i = 0; i < numCells; i++)
			{
				//if( abortFlag ) continue;
				Cell cell = grid->cell( i );
				//debugLog() << points[ cell.index( 0 ) ] << std::endl;
				vertices[i * 12 * 2 + 0] = points[ cell.index( 0 ) ];
				vertices[i * 12 * 2 + 1] = points[ cell.index( 1 ) ];
				vertices[i * 12 * 2 + 2] = points[ cell.index( 1 ) ];
				vertices[i * 12 * 2 + 3] = points[ cell.index( 2 ) ];
				vertices[i * 12 * 2 + 4] = points[ cell.index( 2 ) ];
				vertices[i * 12 * 2 + 5] = points[ cell.index( 3 ) ];
				vertices[i * 12 * 2 + 6] = points[ cell.index( 3 ) ];
				vertices[i * 12 * 2 + 7] = points[ cell.index( 0 ) ];

				vertices[i * 12 * 2 + 8] = points[ cell.index( 4 ) ];
				vertices[i * 12 * 2 + 9] = points[ cell.index( 5 ) ];
				vertices[i * 12 * 2 + 10] = points[ cell.index( 5 ) ];
				vertices[i * 12 * 2 + 11] = points[ cell.index( 6 ) ];
				vertices[i * 12 * 2 + 12] = points[ cell.index( 6 ) ];
				vertices[i * 12 * 2 + 13] = points[ cell.index( 7 ) ];
				vertices[i * 12 * 2 + 14] = points[ cell.index( 7 ) ];
				vertices[i * 12 * 2 + 15] = points[ cell.index( 4 ) ];

				vertices[i * 12 * 2 + 16] = points[ cell.index( 0 ) ];
				vertices[i * 12 * 2 + 17] = points[ cell.index( 7 ) ];
				vertices[i * 12 * 2 + 18] = points[ cell.index( 1 ) ];
				vertices[i * 12 * 2 + 19] = points[ cell.index( 6 ) ];
				vertices[i * 12 * 2 + 20] = points[ cell.index( 2 ) ];
				vertices[i * 12 * 2 + 21] = points[ cell.index( 5 ) ];
				vertices[i * 12 * 2 + 22] = points[ cell.index( 3 ) ];
				vertices[i * 12 * 2 + 23] = points[ cell.index( 4 ) ];
			}

			#pragma omp parallel for
			for (int i = 0; i < vertices.size(); i++)
			{
				//vertices[i] *= 10;
			}

			debugLog() << "Drawing " << vertices.size() << " lines." << std::endl;
			m_gridLines->add( Primitive::LINES ).setColor( color ).setVertices( vertices );
		}

	};

	AlgorithmRegister< ShowGrid > reg( "VisPraktikum/ShowGrid", "Displays input grid" );
}