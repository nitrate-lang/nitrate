# Train a deep neural network

@use "v1.0";
@script;
@import ai;
@import codec;
@import sysio;

enum sex_t {
  male, female,
}

group datum_t {
  # Inputs
  pub age_years: u8,
  pub mass_kg: u16,
  pub sex: sex_t,

  # Outputs
  pub annual_income_dollar: u32,
}

fn quasipure get_data(): std::vecfs<datum_t>? {
  const src_uri = @std::env_param("SOURCE_URI");
  
  var conn = sysio::open(src_uri, "r") catch (e) {
    e.save();
    ret null;
  }

  var rd = conn.reader();
  var parser = codec::live_parser<datum_t>(fn [rd] (): datum_t? {
    let delim = rd.peek();
    if (delim == ',') {
      rd.next();
    } else if (delim == ']') { ret null; }

    let any_obj = codec::json::read_object(rd);

    retz any_obj.has("age", "number"), null;
    retz any_obj.has("mass", "number"), null;
    retz any_obj.has("sex", "bool"), null;
    retz any_obj.has("income", "number"), null;
    
    let x: datum_t;
    x.age_years = any_obj["age"].as_number();
    x.mass = any_obj["mass"].as_number();
    x.sex = any_obj["sex"].as_bool() 
            ? sex_t::male : sex_t::female;
    x.income = any_obj["income"].as_number();
    ret x;
  });

  ret std::vecfs<datum_t>::from_reader(parser)
}

fn train_model(): bool {
  const endpoint = @std::env_param("PROGRESS_HTTP_URI", fn(uri): bool {
    ret codec::is_http_uri(uri, has_params: ["id"]);
  }, "http://localhost:7878/ai/progress.php?id=");

  using ai::model;

  let model = Layout([
    Input((3)),
    Dense((3, 3, 3)),
    Dense((3, 3, 3)),
    Dense((2, 2, 2)),
    Output((1)),
  ]);

  model.emit_progress(fn [endpoint] (data) {
    sysio::post_http(endpoint, 
                     bind_param: {"id": data.ver}, 
                     body: data.to_str()) catch (e) {
      eprintn(e);
      continue;
    };
  });

  let data = get_data() catch (e) {
    e.save();
    ret false;
  };

  model.fit(vecfs: data, cute: true);

  std::again(fn () {
    model.save(@std::emit_file("neural-network"));
  }) catch (e) {
    e.save();
    eprint("Unable to save your neural network");
    ret false;
  };
  
  ret true;
}

std::end(train_model());
