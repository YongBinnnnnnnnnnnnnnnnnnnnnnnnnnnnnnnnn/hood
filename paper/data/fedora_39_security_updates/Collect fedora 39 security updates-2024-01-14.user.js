// ==UserScript==
// @name         Collect fedora 39 security updates
// @namespace    https://github.com/YongBinnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn/hood/
// @version      2024-01-14
// @description  I hate surveillance
// @author       Bin Yong
// @match        https://bodhi.fedoraproject.org/updates/?search=&status=stable&releases=F39&type=security&page=*
// @grant        none
// ==/UserScript==


(function() {
  'use strict';
  let yong = []
  if (window.location.search.slice(-2) != "=1") {
    yong = JSON.parse(localStorage.getItem("yong"));
  }
  let items = document.querySelectorAll("div.list-group .p-1 .ms-2");
  for (const item of items) {
    let record = [
      item.querySelector("a").innerText,
      item.querySelector("span").title,
      item.querySelector("a").href
    ];
    console.log(record);
    yong.push(record);
  }
  localStorage.setItem("yong", JSON.stringify(yong));
  const next_button = document.querySelector("ul.pagination li:last-child a");
  if (next_button.href == "https://bodhi.fedoraproject.org/#") {
    document.head.outerHTML = ""
    document.body.outerHTML = JSON.stringify(yong);
  }
  else {
    next_button.click();
  }
  // Your code here...
})();