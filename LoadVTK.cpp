#include <fstream>
#include <sstream>
#include <string>

#include <fantom/algorithm.hpp>
#include <fantom/register.hpp>
#include <fantom/fields.hpp>

using namespace fantom;

namespace {

	class LoadVTK : public DataAlgorithm {

	std::string m_vtkPath;
	size_t m_dimX, m_dimY, m_dimZ;
	size_t m_numPoints;
	std::vector< float > m_points;
	std::vector< float > m_values;

	bool isVector;	

	public:

		static const bool isAutoRun = true;

		// options
		struct Options : public DataAlgorithm::Options {

			Options( fantom::Options::Control& control ) :
				DataAlgorithm::Options( control )
			{

				add< InputLoadPath >( "Load VTK", "Path to VTK file", "" );

			}

			// process options
			void optionChanged( const std::string& name ) {

			}

		};

		// intput / output
		struct DataOutputs : public DataAlgorithm::DataOutputs {

			DataOutputs( fantom::DataOutputs::Control& control ) :
				DataAlgorithm::DataOutputs( control )
			{

				add< Grid< 3 > >( "grid" );
				add< TensorFieldBase >( "tensor field" );

			}

		};

		// constructor
		LoadVTK( InitData& data ) :
			DataAlgorithm( data )
		{

		}


		void execute( const Algorithm::Options& options, const volatile bool& ) {
			m_points.clear();
			m_values.clear();
			
			m_vtkPath = options.get< InputLoadPath >( "Load VTK" );
			if( m_vtkPath != "" ) {
				parseFile();

				size_t extend[] = { m_dimX, m_dimY, m_dimZ };
				std::vector< Tensor< double, 3 > > gridPoints;
				for( int i=0; i<m_points.size(); i+=3 ) {
					gridPoints.push_back( Tensor< double, 3 >( m_points.at(i), m_points.at(i+1), m_points.at(i+2) ) );
					//if( i == 3 ) debugLog() << m_points.at(i) << std::endl;;
				}
				debugLog() << "grid points size: " << gridPoints.size() << std::endl;
				std::shared_ptr< const DiscreteDomain< 3 > > domain = DomainFactory::makeDomainCurvilinear( extend, gridPoints );

				std::vector< Tensor< double, 3 > > vectors;
				std::vector< Tensor< double, 1 > > scalars;
				if( isVector ) {
					for( int i=0; i<m_values.size(); i+=3 ) {
						vectors.push_back( Tensor< double, 3 >( m_values.at(i), m_values.at(i+1), m_values.at(i+2) ) );
					}
				} else {
					for( int i=0; i<m_values.size(); i++ ) {
						scalars.push_back( Tensor< double, 1 >( m_values.at(i) ) );
					}
				}

				std::shared_ptr< const Grid< 3 > > grid = DomainFactory::makeGridStructured( *domain );
				setResult( "grid", grid );
				if( isVector ) {
					std::shared_ptr< const TensorFieldBase > tensorField = DomainFactory::makeTensorField( *grid, vectors );
					setResult( "tensor field", tensorField );
				} else {
					std::shared_ptr< const TensorFieldBase > tensorField = DomainFactory::makeTensorField( *grid, scalars );
					setResult( "tensor field", tensorField );	
				}

			} else {
				infoLog() << "No input file was selected!" << std::endl;
				return;
			}
		}

	private:

		void parseFile() {

			// read file into string
			std::ifstream inFile( m_vtkPath );
			std::string str;
			getline( inFile, str, char(-1) );
			inFile.close();

			std::vector< std::string > lines;
			split( str, '\n', lines );

			debugLog() << "Number of lines: " << lines.size() << std::endl;

			bool startPointParsing = false;
			bool startValueParsing = false;

			// parse for value type
			for( size_t i=0; i<lines.size(); i++ ) {
				if( lines.at(i).find("VECTORS") != -1 ) {
					isVector = true;
					break;
				} else if( lines.at(i).find("SCALARS") != -1 ) {
					isVector = false;
					break;
				}
			}

			for( size_t i=0; i<lines.size(); i++ ) {

				// read dimensions x, y, z
				if( lines.at(i).find("DIMENSIONS") != -1 ) {
					std::vector< std::string > dimensions;
					split( lines.at(i), ' ', dimensions );

					m_dimX = std::stoi( dimensions[1] );
					m_dimY = std::stoi( dimensions[2] );
					m_dimZ = std::stoi( dimensions[3] );

					debugLog() << "Dimensions: " << m_dimX << " | " << m_dimY << " | " << m_dimZ << std::endl;
				}

				// read num points
				if( lines.at(i).find("POINTS") != -1 ) {
					std::vector< std::string > points;
					split( lines.at(i), ' ', points );
					m_numPoints = std::stoi( points[1] );
					m_points.reserve( m_numPoints );

					startPointParsing = true;

					continue;
				}

				if( lines.at(i).find("VECTORS") != -1 ) {

					startPointParsing = false;
					startValueParsing = true;

					continue;
				} else if( lines.at(i).find("SCALARS") != -1 ) {

					startPointParsing = false;
					startValueParsing = true;

					continue;
				}

				if( lines.at(i).find("POINT_DATA") != -1 ) continue;

				// read points
				if( startPointParsing ) {
					std::vector< std::string > fieldData;
					if( isVector) split( lines.at(i), ' ', fieldData );

					else split( lines.at(i), '\t', fieldData );
					for( size_t j=0; j<fieldData.size(); j++ ) {
						m_points.push_back( std::stof( fieldData.at(j) ) );
					}
				}

				// read vectors
				if( startValueParsing ) {
					std::vector< std::string > vectors;
					if( isVector ) split( lines.at(i), ' ', vectors );
					else split( lines.at(i), '\t', vectors );

					for( int j=0; j<vectors.size(); j++ ) {
						m_values.push_back( std::stof( vectors.at(j) ) );
					}
				}


			}

			debugLog() << "Points size: " << m_points.size() << std::endl;
			debugLog() << "Vector size: " << m_values.size() << std::endl;


		}

		void split( const std::string &s, char delim, std::vector< std::string >& res ) {
			std::stringstream sStream( s );
			std::string item;
			while( std::getline( sStream, item, delim ) ) {
				res.push_back( item );
			}
		}

	};

	AlgorithmRegister< LoadVTK > reg( "VisPraktikum/LoadVTK", "Loads VTK files" );

}
