#include <parser.hpp>
#include <parser_adapt.hpp>

#include <fstream>
#include <iostream>
#include <optional>
#include <vector>
#include <variant>


#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/utility/error_reporting.hpp>

namespace x3 = boost::spirit::x3;

namespace parser {
    struct error_handler
    {
        template <typename Iterator, typename Exception, typename Context>
        x3::error_handler_result on_error(
            Iterator& first, Iterator const& last
        , Exception const& x, Context const& context)
        {
            auto& error_handler = x3::get<x3::error_handler_tag>(context).get();
            std::string message = "Error! Expecting " + x.which() + " here:";
            error_handler(x.where(), message);
            return x3::error_handler_result::fail;
        }
    };
    namespace parsevcd {
        struct str_tag;
        x3::rule<str_tag, std::string> const str = "string";
        auto const str_def = +(x3::lexeme [ +(x3::char_ - '$') ]);

        struct date_tag;
        x3::rule<date_tag, std::string> const date = "date";
        auto const date_def = x3::lit("$date") > str > x3::lit("$end");

        struct version_tag;
        x3::rule<version_tag, std::string> const version = "version";
        auto const version_def = x3::lit("$version") > str > x3::lit("$end");

        struct comment_tag;
        x3::rule<comment_tag, std::string> const comment = "comment";
        auto const comment_def = x3::lit("$comment") > str > x3::lit("$end");

        struct timescale_tag;
        x3::rule<timescale_tag, std::string> const timescale = "timescale";
        auto const timescale_def = x3::lit("$timescale") > str > x3::lit("$end");

        struct scope_tag;
        x3::rule<scope_tag, std::string> const scope = "scope";
        auto const scope_def = x3::lit("$scope") > str > x3::lit("$end");

        struct ident_tag;
        x3::rule<ident_tag, std::string> const ident = "identifier";
        auto const ident_def = x3::lexeme [ +(x3::alnum | x3::punct) > x3::eol ];

        struct signal_tag;
        x3::rule<signal_tag, vcd::Signal> const signal = "signal";
        auto const signal_def = x3::lit("$var") > +x3::alpha > x3::uint_ > x3::lexeme [ +(x3::graph) ] > x3::lexeme [ +(x3::graph) ] > x3::lit("$end");

        struct value_tag;
        x3::rule<value_tag, char> const value = "value";
        auto const value_def = x3::no_case[x3::char_('0') | x3::char_('1') | x3::char_('x') | x3::char_('z') ];

        struct dump_tag;
        x3::rule<dump_tag, vcd::Dump> const dump = "dump";
        auto const dump_def = (x3::lexeme [ ('b' > +value > x3::space) ] | x3::repeat(1)[value]) > ident;

        struct dumpvars_tag;
        x3::rule<dumpvars_tag, std::vector<vcd::Dump>> const dumpvars = "dumpvars";
        auto const dumpvars_def = x3::lit("$dumpvars") > +dump > x3::lit("$end");

        struct timestamp_tag;
        x3::rule<timestamp_tag, vcd::Timestamp> const timestamp = "timestamp";
        auto const timestamp_def = "#" > x3::ulong_long > *dump;

        struct vcd_tag;
        x3::rule<vcd_tag, vcd::Vcd> const vcd = "vcd";
        auto const vcd_def = "" > date > version > -comment > timescale > *scope > +signal > +(x3::lit("$upscope") > x3::lit("$end")) > x3::lit("$enddefinitions") > x3::lit("$end") > dumpvars > +timestamp;

        BOOST_SPIRIT_DEFINE(str,date,version,comment,timescale,scope,ident,signal,value,dump,dumpvars,timestamp,vcd);

        struct vcd_tag : error_handler {};

        template <typename Iterator>
        std::optional<vcd::Vcd> parse_vcd(Iterator first, Iterator last) {
            using x3::ascii::space;
            using x3::eol;


            // using x3::with;
            // using x3::error_handler_tag;
            using error_handler_type = x3::error_handler<Iterator>;

            // // Our error handler
            error_handler_type error_handler(first, last, std::cerr);

            // Our parser
            auto const parserd =
                // we pass our error handler to the parser so we can access
                // it later in our on_error and on_sucess handlers
                x3::with<x3::error_handler_tag>(std::ref(error_handler))
                [
                    parser::parsevcd::vcd
                ];
                // ;

            vcd::Vcd output;
            bool r = x3::phrase_parse(
                first,                          //  Start Iterator
                last,                           //  End Iterator
                parserd,                         //  The Parser
                (space | eol),                  //  The Skip-Parser
                output
            );
            return (r && first == last) ? std::optional<vcd::Vcd>{output} : std::nullopt;
        }

        bool parse() {
            return true;
        }
        
        std::optional<vcd::Vcd> parse_vcd_file(const char* filename) {
            std::ifstream input(filename);
            input.unsetf(std::ios::skipws);
            boost::spirit::istream_iterator begin(input);
            boost::spirit::istream_iterator end;
            return parse_vcd(begin, end);
        }
    }
}
