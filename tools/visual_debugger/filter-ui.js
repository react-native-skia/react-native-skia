// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

const MDCMenu = mdc.menu.MDCMenu;
const MDCMenuFoundation = mdc.menu.MDCMenuFoundation;
const maxZIndex = 999;

class FilterUI extends HTMLElement {
  constructor() {
    super();
  }

  connectedCallback() {
    this.innerHTML = `
<style>
.row {
  display: flex;
  justify-content: space-between;
  padding-bottom: 10px;
}

#filter-container {
  max-width: 400px;
  min-width: 350px;
  font-family: Roboto;
  font-size: 10pt;
  background-color: white;
  padding: 10px;
}

#saveFilter {
  margin: 20px 10px;
  margin-right: 0px;
}

</style>
<div id='filter-container'>
  <div class='sectionTitle' id='filter-ui-title'>Add Filter</div>
  <div class='section'>
    <div class='row'>
      <div class='label' title='Filter annotation to match.
      For example frame.root.damage' >Annotation </div>
        <div class='input'>
          <input placeholder='Substring to match' id='annotation' size=40>
          <!-- TODO: A fancy drop-down here would be nice. -->
        </div>
    </div>
    <div class='row'>
      <div class='label'>File name</div>
      <div class='input'>
        <input placeholder='Substring to match  (usually empty!)'
         id='filename' size=40>
        <!-- TODO: A fancy drop-down here would be nice. -->
      </div>
    </div>
    <div class='row'>
      <div class='label'>Function</div>
      <div class='input'>
        <input placeholder='Substring to match (usually empty!)'
         id='functionname' size=40>
        <!-- TODO: A fancy drop-down here would be nice. -->
      </div>
    </div>


  </div>
  <div style='display: flex'>
    <div style='flex-grow: 1'>
      <div class='sectionTitle'>Action</div>
      <div class='section'>
        <div>
          <form id="actionform">
            <input type='radio' id='drawfilter' name='dowhat' value='drawfilter'/>Override
            <input type='color' id='drawcolor' name='drawcolor' value='#000000'/>
            opacity <input type='range' name='fillalpha' min='0' max='100'
             step='10' value='50' list='alphastep'/>
            <datalist id='alphastep'>
              <option>0</option><option>10</option><option>20</option>
              <option>30</option><option>40</option><option>50</option>
              <option>60</option> <option>70</option><option>80</option>
              <option>90</option><option>100</option>
            </datalist>
            <input checked type='radio' name='dowhat' value='drawcaller'/>
            Draw with caller color and opacity
            <br/>
            <input type='radio' name='dowhat' value='skip'
            title='Used as a discard filter'/>Do not draw
            <br/>
          </form>
        </div>
      </div>
    </div>
    <button id='saveFilter'>Save</button>
  </div>
<div>
`;
    this.setUpButtons_();

    // Event listener to check if custom color selected
    // to automatically select override button
    document.getElementById('drawcolor').addEventListener("change",function(){
      document.getElementById('drawfilter').checked = true;
    });
  }

  setUpButtons_() {
    const button = this.querySelector('#saveFilter');
    button.addEventListener('click', () => {
      let input = this.querySelector('#filename');
      const filename = input.value || undefined;
      input = this.querySelector('#functionname');
      const func = input.value || undefined;
      input = this.querySelector('#annotation');
      const anno = input.value || undefined;

      input = this.querySelector('input[name="dowhat"]:checked');
      const action = { skipDraw: input.value === 'skip' };
      if (input.value === 'drawfilter') {
        action.color = this.querySelector('input[name="drawcolor"]').value;
        action.alpha = this.querySelector('input[name="fillalpha"]').value;
      }

      this.dispatchEvent(new CustomEvent('saveFilter', {
        detail: { selector: { filename, func, anno }, action }
      }));
    });
  }
};

window.customElements.define('filter-ui', FilterUI);

function createFilterChip(filter) {
  const chip = document.createElement('div');
  chip.className = "mdc-chip";
  chip.setAttribute("role", "row");
  chip.style.margin = '5px';
  chip.style.borderRadius = '0px';
  chip.style.zIndex = maxZIndex;
  if (filter.shouldDraw) {
    chip.style.border = filter.drawColor ?
      `2px solid ${filter.drawColor}` : `1px solid black`;
    if (filter.fillAlpha > 0) {
      var alpha = Math.min(.6, parseFloat(filter.fillAlpha) / 100);
      var rgba = filter.drawColor + DrawCall.alphaFloatToHex(alpha);
      chip.style.backgroundColor = rgba;
      chip.style.fontWeight = 'bold';
    }
    else {
      chip.style.backgroundColor = `white`;
    }
  }
  else {
    chip.style.border = `1px dashed grey`;
    chip.style.backgroundColor = `white`;
  }

  chip.innerHTML =
    `
  <input checked type="checkbox"/>
  <span role="gridcell">
  <span class="mdc-chip__text" id="filterstring"></span>
  </span>
  <span role="gridcell">
    <div class="mdc-menu-surface--anchor">
    <i class="material-icons-outlined mdc-chip__icon mdc-chip__icon--trailing"
       tabindex="-1" role="button" id="filtermenu">more_vert</i>
      <div id="filterchipmenu" class="mdc-menu mdc-menu-surface">
        <ul class="mdc-list" role="menu" aria-hidden="true"
         aria-orientation="vertical" tabindex="-1">
          <li class="mdc-list-item" role="menuitem"
          id="filter-moveprev" onclick="movePrev(this)">
            <span class="mdc-list-item__ripple"></span>
            <span class="mdc-list-item__text">
            <i class="material-icons-outlined">arrow_back_ios</i>Move Previous
            </span>
          </li>
          <li class="mdc-list-item" role="menuitem"
          id="filter-movenext" onclick="moveNext(this)">
            <span class="mdc-list-item__ripple"></span>
            <span class="mdc-list-item__text">
            <i class="material-icons-outlined">arrow_forward_ios</i> Move Next
            </span>
          </li>
          <li class="mdc-list-item"
           role="menuitem" id="filter-edit" onclick="showEditFilterPopup(this)">
            <span class="mdc-list-item__ripple"></span>
            <span class="mdc-list-item__text">
            <i class="material-icons-outlined">edit</i> Edit</span>
          </li>
          <li class="mdc-list-item" role="menuitem"
           id="filter-delete" onclick="deleteFilter(this)">
            <span class="mdc-list-item__ripple"></span>
            <span class="mdc-list-item__text">
            <i class="material-icons-outlined">delete</i> Delete</span>
          </li>
        </ul>
      </div>
    </div>
  </span>
  `;

  chip.querySelector('#filterstring').innerHTML = filter.createUIString();

  chip.querySelector('#filtermenu').addEventListener('click', () => {
    const menu = new MDCMenu(chip.querySelector('#filterchipmenu'));
    menu.setAnchorMargin({ top: 25 });
    menu.open = true;
  });

  const check = chip.querySelector('input');
  check.addEventListener('change', () => {
    filter.enabled = !!check.checked;
    Player.instance.refresh();
    Filter.sendStreamFilters();
  });

  return chip;
}

function showCreateFilterPopup(anchor) {
  const filterUi = document.createElement('filter-ui');
  filterUi.addEventListener('saveFilter', (event) => {
    if (event.detail.selector && event.detail.action) {
      const filter = new Filter(event.detail.selector, event.detail.action);
      const chip = createFilterChip(filter);
      const list = document.querySelector('#filters');
      list.appendChild(chip);
      hideModal();
      refreshFilterSet();
    }
  });
  filterUi.style.position = 'absolute';
  filterUi.style.top = (anchor.offsetTop + anchor.offsetHeight) + 'px';
  filterUi.style.left = (anchor.offsetLeft + 20) + 'px';
  filterUi.style.zIndex = maxZIndex;
  
  showModal(filterUi);
}

// Traverses through all the filter chips and enables/disables
// "move next/prev" button clickability depending on if
// the chip is at the beginning/middle/end of list
function refreshFilterSet() {
  const list = document.querySelector('#filters');
  for (var chip = list.firstElementChild;
    chip !== null; chip = chip.nextSibling) {
    if (chip === list.firstElementChild) {
      chip.querySelector('#filter-moveprev')
        .classList.add("mdc-list-item--disabled");
    }
    else {
      chip.querySelector('#filter-moveprev')
        .classList.remove("mdc-list-item--disabled");
    }
    if (chip.nextSibling === null) {
      chip.querySelector('#filter-movenext')
        .classList.add("mdc-list-item--disabled");
    }
    else {
      chip.querySelector('#filter-movenext')
        .classList.remove("mdc-list-item--disabled");
    }
  }
  locallyStoreFilters();

  Player.instance.refresh();

  Filter.sendStreamFilters();
}

function movePrev(item) {
  if (item.classList.contains("mdc-list-item--disabled")) {
    return;
  }
  var chip = item.closest(".mdc-chip");
  const menu = new MDCMenu(chip.querySelector('#filterchipmenu'));
  menu.open = false;
  var index = Array.prototype.indexOf.call(chip.parentNode.children, chip);
  Filter.swapFilters(index - 1, index);
  var prevChip = chip.previousElementSibling;
  chip.parentNode.insertBefore(chip, prevChip);
  refreshFilterSet();
}

function moveNext(item) {
  if (item.classList.contains("mdc-list-item--disabled")) {
    return;
  }
  var chip = item.closest(".mdc-chip");
  const menu = new MDCMenu(chip.querySelector('#filterchipmenu'));
  menu.open = false;
  var index = Array.prototype.indexOf.call(chip.parentNode.children, chip);
  Filter.swapFilters(index, index + 1);
  var nextChip = chip.nextElementSibling;
  chip.parentNode.insertBefore(nextChip, chip);
  refreshFilterSet();
}

function createFilterComplete(selector, action, index) {
  var newFilter =
    new Filter(selector, action, index);
  const newChip = createFilterChip(newFilter);
  const list = document.querySelector('#filters');
  list.appendChild(newChip);
  hideModal();
  refreshFilterSet();
  return newChip;
}

function showEditFilterPopup(item) {
  var chip = item.closest(".mdc-chip");
  var index = Array.prototype.indexOf.call(chip.parentNode.children, chip);

  const menu = new MDCMenu(chip.querySelector('#filterchipmenu'));
  menu.open = false;

  const filterUi = document.createElement('filter-ui');
  filterUi.addEventListener('saveFilter', (event) => {
    if (event.detail.selector && event.detail.action) {
      var newChip = createFilterComplete(event.detail.selector,
        event.detail.action, index);
      chip.replaceWith(newChip);
    }
  });
  filterUi.style.position = 'absolute';
  filterUi.style.top = (chip.offsetTop + chip.offsetHeight) + 'px';
  filterUi.style.left = (chip.offsetLeft + 20) + 'px';
  filterUi.style.zIndex = maxZIndex;

  showModal(filterUi);

  var filter = Filter.getFilter(index);

  // fill form from filter data
  filterUi.querySelector('#filter-ui-title').innerHTML = "Edit Filter";
  filterUi.querySelector('#filename').value = filter.file;
  filterUi.querySelector('#functionname').value = filter.func;
  filterUi.querySelector('#annotation').value = filter.anno;

  var actionform = filterUi.querySelector('#actionform');
  actionform.dowhat.value = !filter.shouldDraw ? 'skip' :
    filter.drawColor ? 'drawfilter' : 'drawcaller';
  actionform.drawcolor.value = filter.drawColor;
  actionform.fillalpha.value = filter.fillAlpha;
}

function deleteFilter(item) {
  var chip = item.closest(".mdc-chip");
  const menu = new MDCMenu(chip.querySelector('#filterchipmenu'));
  menu.open = false;
  var index = Array.prototype.indexOf.call(chip.parentNode.children, chip);
  Filter.deleteFilter(index);
  chip.parentNode.removeChild(chip);
  refreshFilterSet();
}

// Stores filter instances in local storage
function locallyStoreFilters() {
  var filterInstances = Filter.instances;
  // Store string representation of filterInstances list in local storage.
  localStorage.setItem('filterInstances', JSON.stringify(filterInstances));
}

// Restores filter instances from local storage.
function restoreFilters() {
  const retrievedFilterString = localStorage.getItem('filterInstances');
  // Add default filters to the instances list.
  FilterUIDefault.initialize();

  if (!retrievedFilterString) {
    return;
  }

  var retrievedFilterInstances = JSON.parse(retrievedFilterString);
  // Remove any default filter duplicates from restored filters.
  for (const defaultFilter of defaultFilters) {
    retrievedFilterInstances = retrievedFilterInstances.filter
        (instance => !isDuplicate(instance, defaultFilter));
  }
  // Re-create non-default filter chips from local storage.
  // Pre-existing filters are appended behind the default ones.
  retrievedFilterInstances.forEach((instance) =>
    createFilterComplete(instance.selector_, instance.action_));
}

// Checks if one filter is a duplicate of another.
// NOTE: Custom equality check needed to avoid marking same style
// of filters with different enabled states and indices as
// non-duplicates.
function isDuplicate(filter1, filter2) {
  if (!filter1 || !filter2) {
    return false;
  }
  if (filter1.selector_.filename !== filter2.selector_.filename) {
    return false;
  }
  if (filter1.selector_.func !== filter2.selector_.func) {
    return false;
  }
  if (filter1.selector_.anno !== filter2.selector_.anno) {
    return false;
  }
  if (filter1.action_.skipDraw !== filter2.action_.skipDraw) {
    return false;
  }
  if (filter1.action_.color !== filter2.action_.color) {
    return false;
  }
  if (filter1.action_.alpha !== filter2.action_.alpha) {
    return false;
  }
  return true;
}

const defaultFilters = [
    {
      selector_: { filename: "", func: "", anno: "frame.root.quad" },
      action_: { skipDraw: false, color: '#000000', alpha: "10" }
    },
    {
      selector_: { filename: "", func: "", anno: "frame.root.damage" },
      action_: { skipDraw: false, color: '#FF0000', alpha: "20" }
    }
];

// Default filters should probably load off disk.
const FilterUIDefault = {
  initialize() {
    defaultFilters.forEach((instance) =>
      createFilterComplete(instance.selector_, instance.action_))
  }
};

