'use strict'
var cheerio = require('cheerio');

hexo.extend.helper.register('url_for_lang', function(path){
  var root = this.config.root || ''
  var langsArr = this.config.language
  var lang = this.page.lang
  var isDefault = lang === langsArr[0]
  var url = this.url_for(path)

  if (!isDefault && url.indexOf(lang) === -1) {
    url = root + lang + url.replace(root, '/')
  }
  return url
})

hexo.extend.helper.register('page_anchor', function(str){
  var $ = cheerio.load(str, {decodeEntities: false});
  var headings = $('h1, h2, h3, h4, h5, h6');

  if (!headings.length) return str;

  headings.each(function(){
    var id = $(this).attr('id');

    $(this)
      .addClass('article-heading')
      .append('<a class="article-anchor" href="#' + id + '" aria-hidden="true"></a>');
  });

  return $.html();
});

hexo.extend.helper.register('pick_lang', function(lang){
  var root = this.config.root || ''
  var langsArr = this.config.language
  var currentLang = this.page.lang
  var path = this.page.path
  var url = ''

  if (path.split('/')[0] === currentLang) {
    path = path.substring(currentLang.length)
  }

  if (path[0] !== '/') {
    path = '/' + path
  }

  if (lang === langsArr[0] || lang === '') {
    url = root + path.substring(1)
  } else {
    url = root + lang + path
  }

  return url
})