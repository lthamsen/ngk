#include <iostream>
#include <restinio/all.hpp>
#include <json_dto/pub.hpp>
#include <string>


struct place_t
{
	place_t() = default;

	place_t(
		std::string placename,
		double lat,
		double lot )
		:	m_placename{ std::move( placename ) }
		,	m_lat{ std::move( lat ) }
		,	m_lot{ std::move( lot ) }
	{}

	template < typename JSON_IO >
	void
	json_io( JSON_IO & io )
	{
		io
			& json_dto::mandatory( "Placename", m_placename )
			& json_dto::mandatory( "Latitude", m_lat )
			& json_dto::mandatory( "Longitude", m_lot);
	}

	std::string m_placename;
	double m_lat;
	double m_lot;
};

struct weatherStation_t
{
	weatherStation_t() = default;

	weatherStation_t(
		int ID,
		std::string date,
		std::string time,
		place_t place,
		float temperature,
		int humidity )
		:	m_ID{ std::move( ID ) }
		,	m_date{ std::move( date ) }
		,	m_time{ std::move( time ) }
		,	m_place{ std::move( place )}
		,	m_temperature{ std::move( temperature ) }
		,	m_humidity{ std::move( humidity ) }
	{}

	template < typename JSON_IO >
	void
	json_io( JSON_IO & io )
	{
		io
			& json_dto::mandatory( "ID", m_ID )
			& json_dto::mandatory( "Date", m_date )
			& json_dto::mandatory( "Time", m_time )
			& json_dto::mandatory( "Place", m_place )
			& json_dto::mandatory( "Temperature", m_temperature )
			&json_dto::mandatory( "Humidity", m_humidity);
	}

	int m_ID;
	std::string m_date;
	std::string m_time;
	place_t m_place;
	float m_temperature;
	int m_humidity;

	
};

using weatherReg = std::vector< weatherStation_t >;

namespace rr = restinio::router;
using router_t = rr::express_router_t<>;

class weatherStation_handler_t
{
public :
	explicit weatherStation_handler_t( weatherReg & weather )
		:	m_weather( weather )
	{}

	weatherStation_handler_t( const weatherStation_handler_t & ) = delete;
	weatherStation_handler_t( weatherStation_handler_t && ) = delete;

	auto on_weather_data_list(
		const restinio::request_handle_t& req, rr::route_params_t ) const
	{
		auto resp = init_resp( req->create_response() );

		resp.set_body(
			"Weather data (data count: " +
				std::to_string( m_weather.size() ) + ")\n" );

		for( std::size_t i = 0; i < m_weather.size(); ++i )
		{
			resp.append_body( std::to_string( i + 1 ) + ". " );
			const auto & b = m_weather[ i ];
			resp.append_body( std::to_string(b.m_ID) + ", " + b.m_date + ", " 
			+ b.m_time + ", [" + b.m_place.m_placename + ", " + std::to_string(b.m_place.m_lat) + ", " 
			+ std::to_string(b.m_place.m_lat) + "]," + std::to_string(b.m_temperature)+ ", " + std::to_string(b.m_humidity) +"\n");
		}

		return resp.done();
	}

	auto on_weather_data_get_latest(
		const restinio::request_handle_t& req, rr::route_params_t params )
	{

		auto resp = init_resp( req->create_response() );

		for(size_t i = m_weather.size()-1; i >=0 && i >= m_weather.size()-3; i--)
		{
			resp.append_body( std::to_string( i + 1 ) + ". " );
			const auto & b = m_weather[ i ];
			resp.append_body( std::to_string(b.m_ID) + ", " + b.m_date + ", " 
			+ b.m_time + ", [" + b.m_place.m_placename + ", " + std::to_string(b.m_place.m_lat) + ", " 
			+ std::to_string(b.m_place.m_lat) + "]," + std::to_string(b.m_temperature)+ ", "+ std::to_string(b.m_humidity) +"\n");	
		}

		return resp.done();
	}

	auto on_new_weather_data(
		const restinio::request_handle_t& req, rr::route_params_t )
	{
		auto resp = init_resp( req->create_response() );

		try
		{
			m_weather.emplace_back(
				json_dto::from_json< weatherStation_t >( req->body() ) );
		}
		catch( const std::exception & ) //OBS!!!!!!!!
		{
			mark_as_bad_request( resp );
		}

		return resp.done();
	}

	auto on_date_get(
		const restinio::request_handle_t& req, rr::route_params_t params )
	{
		auto resp = init_resp( req->create_response() );
		try
		{
			auto date = restinio::utils::unescape_percent_encoding( params[ "date" ] );

			resp.set_body( "Weather on " + date + ":\n" );

			for( std::size_t i = 0; i < m_weather.size(); ++i )
			{
				const auto & b = m_weather[ i ];
				if( date == b.m_date )
				{
					resp.append_body( std::to_string(b.m_ID) + ", " + b.m_date + ", " 
					+ b.m_time + ", [" + b.m_place.m_placename + ", " + std::to_string(b.m_place.m_lat) + ", " 
					+ std::to_string(b.m_place.m_lat) + "]," + std::to_string(b.m_temperature) + ", " + std::to_string(b.m_humidity) +"\n");	
				}
			}
		}
		catch( const std::exception & )
		{
			mark_as_bad_request( resp );
		}

		return resp.done();
	}
	auto on_weather_update(
		const restinio::request_handle_t& req, rr::route_params_t params )
	{
		const auto weatherID = restinio::cast_to< std::uint32_t >( params[ "weather_id" ] );

		auto resp = init_resp( req->create_response() );

		try
		{
			auto b = json_dto::from_json< weatherStation_t >( req->body() );

			if( 0 != weatherID && weatherID <= m_weather.size() )
			{
				m_weather[ weatherID - 1 ] = b;
			}
			else
			{
				mark_as_bad_request( resp );
				resp.set_body( "No weather data with id #" + std::to_string( weatherID ) + "\n" );
			}
		}
		catch( const std::exception &  )
		{
			mark_as_bad_request( resp );
		}

		return resp.done();
	}
/*

*/
/*

	auto on_book_delete(
		const restinio::request_handle_t& req, rr::route_params_t params )
	{
		const auto weatherID = restinio::cast_to< std::uint32_t >( params[ "weatherID" ] );

		auto resp = init_resp( req->create_response() );

		if( 0 != weatherID && weatherID <= m_weather.size() )
		{
			const auto & b = m_weather[ weatherID - 1 ];
			resp.set_body(
				"Delete book #" + std::to_string( weatherID ) + ": " +
					b.m_title + "[" + b.m_author + "]\n" );

			m_weather.erase( m_weather.begin() + ( weatherID - 1 ) );
		}
		else
		{
			resp.set_body(
				"No book with #" + std::to_string( weatherID ) + "\n" );
		}

		return resp.done();
	}
*/
private :
	weatherReg & m_weather;

	template < typename RESP >
	static RESP
	init_resp( RESP resp )
	{
		resp
			.append_header( "Server", "RESTinio sample server /v.0.6" )
			.append_header_date_field()
			.append_header( "Content-Type", "text/plain; charset=utf-8" );

		return resp;
	}

	template < typename RESP >
	static void
	mark_as_bad_request( RESP & resp )
	{
		resp.header().status_line( restinio::status_bad_request() );
	}
};

auto server_handler( weatherReg & weather_data )
{
	auto router = std::make_unique< router_t >();
	auto handler = std::make_shared< weatherStation_handler_t >( std::ref(weather_data) );

	auto by = [&]( auto method ) {
		using namespace std::placeholders;
		return std::bind( method, handler, _1, _2 );
	};

	auto method_not_allowed = []( const auto & req, auto ) {
			return req->create_response( restinio::status_method_not_allowed() )
					.connection_close()
					.done();
		};

	// Handlers for '/' path.
	router->http_get( "/", by( &weatherStation_handler_t::on_weather_data_list ) );
	router->http_get( "/latest", by( &weatherStation_handler_t::on_weather_data_get_latest ) );
	router->http_get( "/date/:date", by( &weatherStation_handler_t::on_date_get ) );
	router->http_post( "/", by( &weatherStation_handler_t::on_new_weather_data ) );

	router->http_put("/:weather_id",  by( &weatherStation_handler_t::on_weather_update ) );
	// Disable all other methods for '/'.
	router->add_handler(
			restinio::router::none_of_methods(
					restinio::http_method_get(), restinio::http_method_post() ),
			"/", method_not_allowed );

	// Handler for '/author/:author' path.

	// Disable all other methods for '/author/:author'.
	//router->add_handler(
	//		restinio::router::none_of_methods( restinio::http_method_get() ),
	//		"/author/:author", method_not_allowed );
/*
	// Handlers for '/:weatherID' path.
	router->http_get(
			R"(/:weatherID(\d+))",
			by( &weatherStation_handler_t::on_book_get ) );
	router->http_delete(
			R"(/:weatherID(\d+))",
			by( &weatherStation_handler_t::on_book_delete ) );

	// Disable all other methods for '/:weatherID'.
	router->add_handler(
			restinio::router::none_of_methods(
					restinio::http_method_get(),
					restinio::http_method_post(),
					restinio::http_method_delete() ),
			R"(/:weatherID(\d+))", method_not_allowed );
*/
	return router;
}

int main()
{
	using namespace std::chrono;

	try
	{
		using traits_t =
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				restinio::single_threaded_ostream_logger_t,
				router_t >;

		 weatherReg weather_data{};

		restinio::run(
			restinio::on_this_thread< traits_t >()
				.address( "localhost" )
				.request_handler( server_handler( weather_data ) )
				.read_next_http_message_timelimit( 10s )
				.write_http_response_timelimit( 1s )
				.handle_request_timeout( 1s ) );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
