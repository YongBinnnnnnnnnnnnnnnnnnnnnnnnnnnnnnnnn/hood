// ==UserScript==
// @name         Collect Apple security releases
// @namespace    https://github.com/YongBinnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn/hood/
// @version      2024-01-10
// @description  I hate surveillance.
// @author       Bin Yong
// @match        https://support.apple.com/en-us/HT201222
// @grant        GM_getResourceText
// ==/UserScript==

(async function() {
  'use strict';
  const links = document.querySelectorAll("#tableWraper a");
  const target_products = [
    "watchOS",
    "iOS",
    "iPadOS",
    "macOS",
    "tvOS",
  ];
  const renderer = document.querySelector("div.footer-wrapper");
  unsafeWindow.yong = {};
  for (const link of links) {
    let is_target_product = false;
    const name = link.textContent;
    for (const product of target_products) {
      if (name.indexOf(product) != -1) {
        is_target_product = true;
      }
    }
    if (!is_target_product) {
      continue;
    }
    //if (name.indexOf("macOS Ventura 13.4.1 (a)") == -1) {
    //  continue;
    //}
    console.log(name);
    console.log(link.href);
    const response = await fetch(link.href);
    let update_text = await response.text();
    if (update_text.indexOf('<section class="section section-content">') != -1) {
      update_text ='<section class="section section-content">' +  update_text.split('<section class="section section-content">')[1];
      update_text = update_text.split('</section>')[0] + '</section>';
    }
    else {
      update_text ='<div id="sections">' +  update_text.split('<div id="sections">')[1];
      update_text = update_text.split("<div id='helpful-rating-wrapper'>")[0];
      update_text = update_text.substr(0, update_text.lastIndexOf("</div>"));
    }
    renderer.innerHTML = update_text;
    update_text = renderer.innerText;
    //update_text = update_text.replace(/\s\s+/gm, " ");
    //update_text = update_text.replace(/\n\n/gm, "\n");
    //console.log(update_text);
    unsafeWindow.yong[name] = update_text;
  }
  document.querySelector('html').outerHTML = JSON.stringify(unsafeWindow.yong);
})();
// count word freq
