/* jshint browser: true, devel: true */

/**
 * preg_replace (from PHP) in JavaScript!
 *
 * This is basically a pattern replace. You can use a regex pattern to search and
 * another for the replace. For more information see the PHP docs on the original
 * function (http://php.net/manual/en/function.preg-replace.php), and for more on
 * JavaScript flavour regex visit http://www.regular-expressions.info/javascript.html
 *
 * NOTE: Unlike the PHP version, this function only deals with string inputs. No arrays.
 *
 * @author	William Duyck <fuzzyfox0@gmail.com>
 * @license http://www.mozilla.org/MPL/2.0/ Mozilla Public License 2.0
 *
 * @param	{String}	pattern	The pattern to search for.
 * @param	{String}	replace	The string to replace.
 * @param	{String}	subject	The string to search and replace.
 * @param	{Integer}	limit	The maximum possible replacements.
 * @return	{String}	If matches are found, the new subject will be returned.
 */
var preg_replace=function(a,b,c,d){void 0===d&&(d=-1);var e=a.substr(a.lastIndexOf(a[0])+1),f=a.substr(1,a.lastIndexOf(a[0])-1),g=RegExp(f,e),i=[],j=0,k=0,l=c,m=[];if(-1===d){do m=g.exec(c),null!==m&&i.push(m);while(null!==m&&-1!==e.indexOf("g"))}else i.push(g.exec(c));for(j=i.length-1;j>-1;j--){for(m=b,k=i[j].length;k>-1;k--)m=m.replace("${"+k+"}",i[j][k]).replace("$"+k,i[j][k]).replace("\\"+k,i[j][k]);l=l.replace(i[j][0],m)}return l};

/**
 * Basic Markdown Parser
 *
 * This function parses a small subset of the Markdown language as defined by
 * [John Gruber](http://daringfireball.net/projects/markdown). It's very basic
 * and needs to be refactored a little, and there are plans to add more support
 * for the rest of the language in the near future.
 *
 * This implimentation is based loosely on
 * [slimdown.php](https://gist.github.com/jbroadway/2836900) by Johnny Broadway.
 *
 * @version 0.1
 * @author William Duyck <fuzzyfox0@gmail.com>
 * @license http://www.mozilla.org/MPL/2.0/ Mozilla Public License 2.0
 *
 * @param  {String} str A Markdown string to be converted to HTML.
 * @return {String}     The HTML for the given Markdown.
 */
var markdown_parser = function(str){

    var rules = [
        // headers
        ['/(#+)(.*)/g', function(chars, header){
            var level = chars.length;
            return '<h'+level+'>'+header.trim()+'</h'+level+'>';
        }],
        // images
        ['/\\!\\[([^\\[]+)\\]\\(([^\\(]+)\\)/g', '<img src=\"\\2\" alt=\"\\1\" />'],
        // link
        ['/\\[([^\\[]+)\\]\\(([^\\(]+)\\)/g', '<a href=\"\\2\">\\1</a>'],
        // bold
        ['/(\\*\\*|__)(.*?)\\1/g', '<strong>\\2</strong>'],
        // emphasis
        ['/(\\*|_)(.*?)\\1/g', '<em>\\2</em>'],
        // strike
        ['/(\\~\\~)(.*?)\\1/g', '<del>\\2</del>'],
        // quote
        ['/\\:\\"(.*?)\\"\\:/g', '<q>\\1</q>'],
        // unordered list
        ['/\\n\\*(.*)/g', function(item){
            return '<ul>\n<li>'+item.trim()+'</li>\n</ul>';
        }],
        // ordered list
        ['/\\n[0-9]+\\.(.*)/g', function(item){
            return '<ol>\n<li>'+item.trim()+'</li>\n</ol>';
        }],
        // blockquote
        ['/\\n\\>(.*)/g', function(str){
            return '<blockquote>'+str.trim()+'</blockquote>';
        }],
        // paragraphs
        ['/\\n[^\\n]+\\n/g', function(line){
            line = line.trim();
            if(line[0] === '<'){
                return line;
            }
            return '\n<p>'+line+'</p>\n';
        }]
    ], fixes = [
        ['/<\\/ul>\n<ul>/g', '\n'],
        ['/<\\/ol>\n<ol>/g', '\n'],
        ['/<\\/blockquote>\n<blockquote>/g', "\n"]
    ];

    var parse_line = function(str){
        str = "\n" + str.trim() + "\n";
        for(var i = 0, j = rules.length; i < j; i++){
            if(typeof rules[i][1] == 'function') {
                var _flag = rules[i][0].substr(rules[i][0].lastIndexOf(rules[i][0][0])+1),
                    _pattern = rules[i][0].substr(1, rules[i][0].lastIndexOf(rules[i][0][0])-1),
                    reg	= new RegExp(_pattern, _flag);

                var matches = reg.exec(str);
                if(matches !== null){
                    if(matches.length > 1){
                        str = preg_replace(rules[i][0], rules[i][1](matches[1], matches[2]), str);
                    }
                    else
                    {
                        str = preg_replace(rules[i][0], rules[i][1](matches[0]), str);
                    }
                }
            }
            else {
                str = preg_replace(rules[i][0], rules[i][1], str);
            }
        }
        return str.trim();
    };

    str = str.split('\n');
    var rtn = [];
    for(var i = 0, j = str.length; i < j; i++){
        rtn.push(parse_line(str[i]));
    }

    rtn = rtn.join('\n');

    for(i = 0, j = fixes.length; i < j; i++){
        rtn = preg_replace(fixes[i][0], fixes[i][1], rtn);
    }

    return rtn;
};
