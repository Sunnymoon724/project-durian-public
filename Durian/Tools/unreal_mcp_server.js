const readline = require('readline');
const http = require('http');

const base = 'http://127.0.0.1:30010';
const tools = [
  { name: 'unreal_status', description: 'Check Unreal Remote Control API.', inputSchema: { type: 'object', properties: {} } },
  { name: 'unreal_get_preset', description: 'Read an Unreal Remote Control Preset.', inputSchema: { type: 'object', properties: { preset: { type: 'string' } }, required: ['preset'] } },
  { name: 'unreal_set_property', description: 'Set an exposed Unreal property.', inputSchema: { type: 'object', properties: { object_path: { type: 'string' }, property_name: { type: 'string' }, property_value: {} }, required: ['object_path', 'property_name', 'property_value'] } }
];

function request(method, path, body) {
  return new Promise((resolve, reject) => {
    const data = body === undefined ? null : Buffer.from(JSON.stringify(body));
    const req = http.request(base + path, { method, headers: data ? { 'Content-Type': 'application/json', 'Content-Length': data.length } : {} }, res => {
      let raw = ''; res.setEncoding('utf8');
      res.on('data', x => raw += x);
      res.on('end', () => { try { resolve(JSON.parse(raw)); } catch { resolve(raw); } });
    });
    req.on('error', reject); req.setTimeout(5000, () => req.destroy(new Error('Unreal API timeout')));
    if (data) req.write(data); req.end();
  });
}
function out(id, result) { process.stdout.write(JSON.stringify({ jsonrpc: '2.0', id, result }) + '\n'); }
async function handle(m) {
  if (m.method === 'initialize') return out(m.id, { protocolVersion: '2024-11-05', capabilities: { tools: {} }, serverInfo: { name: 'durian-unreal', version: '0.1.0' } });
  if (m.method === 'tools/list') return out(m.id, { tools });
  if (m.method === 'notifications/initialized') return;
  if (m.method !== 'tools/call') return out(m.id, {});
  const a = m.params.arguments || {}, name = m.params.name;
  try {
    let value;
    if (name === 'unreal_status') value = await request('GET', '/remote/info');
    else if (name === 'unreal_get_preset') value = await request('GET', '/remote/preset/' + encodeURIComponent(a.preset));
    else if (name === 'unreal_set_property') value = await request('PUT', '/remote/object/property', { ObjectPath: a.object_path, PropertyName: a.property_name, PropertyValue: a.property_value });
    else throw new Error('Unknown tool: ' + name);
    out(m.id, { content: [{ type: 'text', text: JSON.stringify(value, null, 2) }] });
  } catch (e) { out(m.id, { isError: true, content: [{ type: 'text', text: e.message }] }); }
}
readline.createInterface({ input: process.stdin }).on('line', line => { try { handle(JSON.parse(line)); } catch (e) {} });
