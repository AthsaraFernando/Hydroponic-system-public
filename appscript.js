function doPost(e) {
    const sheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();
    const data = JSON.parse(e.postData.contents);
    sheet.appendRow([
      new Date(),
      data.temperature,
      data.humidity,
      data.light,
      data.water,
    ]);
    return ContentService.createTextOutput("Data added successfully");
  }