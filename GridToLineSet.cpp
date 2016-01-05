#include <fantom/algorithm.hpp>
#include <fantom/register.hpp>
#include <fantom/fields.hpp>
#include <fantom/datastructures/LineSet.hpp>

using namespace fantom;

namespace {

	class GridToLineSet : public DataAlgorithm {

	private:
		std::shared_ptr< const Grid< 3 > > m_grid;
		std::shared_ptr< LineSet > m_streamlines;

	public:
		static const bool isAutoRun = true;

		struct Options : public DataAlgorithm::Options {
			Options( fantom::Options::Control& control ) :
				DataAlgorithm::Options( control )
			{
				add< Grid< 3 > >( "Grid", "3D input grid of line celltype" );
			}
		};

		struct DataOutputs : public DataAlgorithm::DataOutputs {
			DataOutputs( fantom::DataOutputs::Control& control ) :
				DataAlgorithm::DataOutputs( control )
			{
				add< LineSet >( "Streamlines" );
			}
		};

		GridToLineSet( InitData& data ) :
			DataAlgorithm( data )
		{

		}

		virtual void execute( const Algorithm::Options& options, const volatile bool& abortFlag ) override {

			m_grid = options.get< Grid< 3 > >( "Grid" );
			if( !m_grid ) {
				infoLog() << "No input grid!" << std::endl;
				return;
			}

			if( m_grid->getMaximalCellDimension() > 1 || m_grid->numCellTypes() > 1 ) {
				infoLog() << "Input grid doesn't exclusively consist of line cells!" << std::endl;
				return;
			}

			m_streamlines = std::shared_ptr< LineSet >( new LineSet );
			const ValueArray< Point3 >& points = m_grid->points();

			std::vector< Point3 > vertices;
			std::vector< size_t > lines;

			//#pragma omp parallel for
			for( int i=0; i<m_grid->numCells(); i++ ) {
				Cell cell = m_grid->cell( i );
				vertices.push_back( points[ cell.index( 0 ) ] );
				vertices.push_back( points[ cell.index( 1 ) ] );
				if( i < m_grid->numCells()-1 ) {
					Cell nextCell = m_grid->cell( i+1 );
					if( points[ cell.index(1) ] != points[ nextCell.index(0) ] ) lines.push_back( vertices.size() );
				}
			}

			int offset = 0;
			for( int i=0; i<lines.size(); i++ ) {
				std::vector< size_t > indices;
				for( int j=offset; j<lines.at(i); j++ ) {
					indices.push_back( m_streamlines->addPoint( vertices.at( j ) ) );
				}
				offset = lines.at(i);
				m_streamlines->addLine( indices );
			}

			setResult( "Streamlines", m_streamlines );

		}

	};

	AlgorithmRegister< GridToLineSet > reg( "VisPraktikum/GridToLineSet", "Converts a grid with line cells into a LineSet" );

}